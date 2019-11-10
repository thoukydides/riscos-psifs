/*
    File        : fontobj.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Font handling for the PsiFS filer.

    License     : PsiFS is free software: you can redistribute it and/or
                  modify it under the terms of the GNU General Public License
                  as published by the Free Software Foundation, either
                  version 3 of the License, or (at your option) any later
                  version.
    
                  PsiFS is distributed in the hope that it will be useful,
                  but WITHOUT ANY WARRANTY; without even the implied warranty
                  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
                  the GNU General Public License for more details.
    
                  You should have received a copy of the GNU General Public
                  License along with PsiFS. If not, see
                  <http://www.gnu.org/licenses/>.
*/

// Include header file for this module
#include "fontobj.h"

// Include clib header files
#include <limits.h>
#include <stdio.h>

// Include cathlibcpp
#include "list.h"

// Include oslib header files
#include "oslib/colourtrans.h"

// Include project header files
#include "options.h"

// Options tags
static const char fontobj_tag_font[] = "Font";
static const char fontobj_tag_bold[] = "Bold";
static const char fontobj_tag_italic[] = "Italic";

// Default underline and strikethough settings (1/256 em)
#define FONTOBJ_UNDERLINE_POSITION (-25)
#define FONTOBJ_UNDERLINE_THICKNESS (15)
#define FONTOBJ_STRIKETHROUGH_POSITION (80)

// Range of baseline ratios for full justification
#define FONTOBJ_JUSTIFY_MIN (0.94)
#define FONTOBJ_JUSTIFY_MAX (1.10)

// Miscellaneous font metrics
struct fontobj_misc_info
{
    int x0;
    int y0;
    int x1;
    int y1;
    int xkern;
    int ykern;
    int italic_correction;
    byte underline_position;
    byte underline_thickness;
    int cap_height;
    int xheight;
    int descender;
    int ascender;
    int reserved;
};

// Default font mappings
struct fontobj_default_map_struct
{
    const char *tag;
    const char *value;
};
static const fontobj_default_map_struct fontobj_default_map[] =
{
    {"Font",                            "Corpus.Medium"},
    {"Font_Arial",                      "Homerton.Medium"},
    {"Font_Arial_Bold",                 "Homerton.Bold"},
    {"Font_Arial_Bold_Italic",          "Homerton.Bold.Oblique"},
    {"Font_Arial_Italic",               "Homerton.Medium.Oblique"},
    {"Font_CourierNew",                 "Corpus.Medium"},
    {"Font_CourierNew_Bold",            "Corpus.Bold"},
    {"Font_CourierNew_Bold_Italic",     "Corpus.Bold.Oblique"},
    {"Font_CourierNew_Italic",          "Corpus.Medium.Oblique"},
    {"Font_TimesNewRoman",              "Trinity.Medium"},
    {"Font_TimesNewRoman_Bold",         "Trinity.Bold"},
    {"Font_TimesNewRoman_Bold_Italic",  "Trinity.Bold.Italic"},
    {"Font_TimesNewRoman_Italic",       "Trinity.Medium.Italic"},
    {"Font_Screen0",                    "Corpus.Medium"},
    {"Font_Screen0_Bold",               "Corpus.Bold"},
    {"Font_Screen0_Bold_Italic",        "Corpus.Bold.Oblique"},
    {"Font_Screen0_Italic",             "Corpus.Medium.Oblique"},
    {"Font_Screen1",                    "Homerton.Medium"},
    {"Font_Screen1_Bold",               "Homerton.Bold"},
    {"Font_Screen1_Bold_Italic",        "Homerton.Bold.Oblique"},
    {"Font_Screen1_Italic",             "Homerton.Medium.Oblique"},
    {"Font_Screen2",                    "Corpus.Medium"},
    {"Font_Screen2_Bold",               "Corpus.Bold"},
    {"Font_Screen2_Bold_Italic",        "Corpus.Bold.Oblique"},
    {"Font_Screen2_Italic",             "Corpus.Medium.Oblique"},
    {"Font_Screen3",                    "Trinity.Medium"},
    {"Font_Screen3_Bold",               "Trinity.Bold"},
    {"Font_Screen3_Bold_Italic",        "Trinity.Bold.Italic"},
    {"Font_Screen3_Italic",             "Trinity.Medium.Italic"}
};
#define FONTOBJ_DEFAULT_MAP_SIZE (sizeof(fontobj_default_map) / sizeof(fontobj_default_map_struct))

// Screen font format string
#define FONTOBJ_SCREEN "Screen%i"

// Maximum number of font handles
#define FONTOBJ_MAX_HANDLES (16)

// List of active font handles
static list<fontobj_base *> fontobj_list;

/*
    Parameters  : font              - The name of the font.
                  size              - The size of the font in sixteenths
                                      of a point.
    Returns     : -
    Description : Constructor.
*/
fontobj_base::fontobj_base(const string &font, int size)
{
    os_error *err = NULL;

    // Store the font information
    this->font = font;
    this->size = size;

    // Find the font
    err = xfont_find_font(font.c_str(), size, size, 0, 0, &handle, NULL, NULL);
    if (err) handle = font_SYSTEM;

    // Read the font metrics
    int info_size;
    if (!err)
    {
        err = xfont_read_font_metrics(handle, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL, &info_size, NULL);
    }
    if (!err && (info_size == sizeof(fontobj_misc_info)))
    {
        fontobj_misc_info info;
        font_read_font_metrics(handle, NULL, NULL, NULL,
                               (font_misc_info *) &info, NULL,
                               NULL, NULL, NULL, NULL, NULL, NULL);
        underline = TRUE;
        underline_position = (char) info.underline_position;
        underline_thickness = info.underline_thickness;
    }
    else
    {
        underline = FALSE;
        underline_position = underline_thickness = 0;
    }

    // Reference count starts at one
    count = 1;

    // Add to the list of font handles
    fontobj_list.push_back((fontobj_base *) this);
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
fontobj_base::~fontobj_base()
{
    // Remove from the list of font handles
    fontobj_list.remove((fontobj_base *) this);

    // Lose the font
    xfont_lose_font(handle);
}

/*
    Parameters  : void
    Returns     : void
    Description : Increment the reference count.
*/
void fontobj_base::inc()
{
    // Increment the reference count
    count++;

    // Shuffle to the back of the list
    list_iterator<fontobj_base *> i = fontobj_list.begin();
    while ((i != fontobj_list.end()) && (*i != (fontobj_base *) this))
    {
        ++i;
    }
    if (i != fontobj_list.end())
    {
        fontobj_list.splice(fontobj_list.end(), fontobj_list, i);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Decrement the reference count.
*/
void fontobj_base::dec()
{
    // Decrease the reference count, and delete if appropriate
    if (!--count) delete this;
}

/*
    Parameters  : void
    Returns     : string            - The name of the font.
    Description : Return the name of the font.
*/
string fontobj_base::get_font() const
{
    return font;
}

/*
    Parameters  : void
    Returns     : int               - The size of the font.
    Description : Return the size of the font.
*/
int fontobj_base::get_size() const
{
    return size;
}

/*
    Parameters  : void
    Returns     : font_f            - The font handle.
    Description : Return the raw font handle.
*/
fontobj_base::operator font_f() const
{
    return handle;
}

/*
    Parameters  : position          - The underline position in 1/256 em.
                  thickness         - The underline thickness in 1/256 em.
    Returns     : bool              - Is underline information available.
    Description : Read the recommended underline position and thickness
                  from the font metrics.
*/
bool fontobj_base::get_underline(int &position, int &thickness) const
{
    // Return the underline information
    position = underline_position;
    thickness = underline_thickness;
    return underline;
}

/*
    Parameters  : font              - The name of the font.
                  size              - The size of the font in sixteenths
                                      of a point.
    Returns     : -
    Description : Obtain a font handle.
*/
fontobj_base *fontobj_base::find(const string &font, int size)
{
    // Attempt to find an existing font handle
    list_iterator<fontobj_base *> i = fontobj_list.begin();
    while ((i != fontobj_list.end())
           && (((*i)->font != font) || ((*i)->size != size)))
    {
        ++i;
    }

    // Check whether a handle was found
    fontobj_base *handle;
    if (i == fontobj_list.end())
    {
        // Attempt to lose handles if there are too many
        i = fontobj_list.begin();
        while ((i != fontobj_list.end())
               && (FONTOBJ_MAX_HANDLES < fontobj_list.size()))
        {
            // Only interested in fonts with a reference count of one
            handle = *i++;
            if (handle->count == 1) handle->dec();
        }

        // Create a new handle
        handle = new fontobj_base(font, size);
    }
    else handle = *i;

    // Return the handle
    return handle;
}

/*
    Parameters  : void
    Returns     : void
    Description : Lose all fonts.
*/
void fontobj_base::lose_all()
{
    // Remove all elements from the list and decrement their reference counts
    while (!fontobj_list.empty())
    {
        fontobj_base *handle = fontobj_list.front();
        fontobj_list.pop_front();
        handle->dec();
    }
}

/*
    Parameters  : font              - The name of the font.
                  size              - The size of the font in internal
                                      units.
    Returns     : -
    Description : Constructor.
*/
fontobj_handle::fontobj_handle(const string &font, int size)
{
    // Store the font size
    this->size = size;

    // Obtain a handle for this font
    ref = fontobj_base::find(font, transform_to_point16(size));

    // Increase the reference count for this handle
    ref->inc();
}

/*
    Parameters  : handle            - The font handle.
    Returns     : -
    Description : Constructor.
*/
fontobj_handle::fontobj_handle(const fontobj_handle &handle)
{
    // Increase the reference count of the handle being copied
    handle.ref->inc();

    // Copy the font size and handle, and increase the reference count
    size = handle.size;
    ref = handle.ref;
}

/*
    Parameters  : handle            - The font handle to copy.
    Returns     : fontobj_handle    - A reference to this handle.
    Description : Handle assignment.
*/
fontobj_handle &fontobj_handle::operator=(const fontobj_handle &handle)
{
    // Increase the reference count for the handle being copied
    handle.ref->inc();

    // Reduce the reference count of this handle
    ref->dec();

    // Copy the font size and handle
    size = handle.size;
    ref = handle.ref;

    // Return a reference to this handle
    return *this;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
fontobj_handle::~fontobj_handle()
{
    // Decrement the reference count of this handle
    ref->dec();
}

/*
    Parameters  : void
    Returns     : font_f            - The font handle.
    Description : Return the raw font handle.
*/
fontobj_handle::operator font_f() const
{
    return font_f(*ref);
}

/*
    Parameters  : bottom            - Offset from baseline to bottom of
                                      underline in internal units.
                  top               - Offset from baseline to top of
                                      underline in internal units.
    Returns     : void
    Description : Obtain the recommended underline position.
*/
void fontobj_handle::get_underline(int &bottom, int &top) const
{
    // Get the recommended underline position and thickness
    int position;
    int thickness;
    if (!ref->get_underline(position, thickness))
    {
        position = FONTOBJ_UNDERLINE_POSITION;
        thickness = FONTOBJ_UNDERLINE_THICKNESS;
    }

    // Convert to internal units
    top = transform_base::round((position / 256.0) * size);
    bottom = transform_base::round(((position - thickness) / 256.0) * size);
}

/*
    Parameters  : bottom            - Offset from baseline to bottom of
                                      strikethrough in internal units.
                  top               - Offset from baseline to top of
                                      strikethrough in internal units.
    Returns     : void
    Description : Obtain the recommended strikethrough position.
*/
void fontobj_handle::get_strikethrough(int &bottom, int &top) const
{
    // Get the recommended strikethrough position and thickness
    int position;
    int thickness;
    if (!ref->get_underline(position, thickness))
    {
        thickness = FONTOBJ_UNDERLINE_THICKNESS;
    }
    position = FONTOBJ_STRIKETHROUGH_POSITION;

    // Convert to internal units
    top = transform_base::round((position / 256.0) * size);
    bottom = transform_base::round(((position - thickness) / 256.0) * size);
}

/*
    Parameters  : text              - The text to process.
                  left              - The start position in internal units.
                  spacing           - Additional spacing between characters.
                  matrix            - Font transformation matrix.
    Returns     : os_box            - The bounding box in internal units.
    Description : Calculate the bouning box for the specified text.
*/
os_box fontobj_handle::get_box(const string &text, const os_coord &left,
                               int spacing, const os_trfm *matrix) const
{
    // Construct a scan block for the additional spacing
    font_scan_block block;
    block.space.x = transform_to_millipoint(spacing);
    block.space.y = 0;
    block.letter.x = transform_to_millipoint(spacing);
    block.letter.y = 0;
    block.split_char = -1;
    block.bbox.x0 = block.bbox.y0 = block.bbox.x1 = block.bbox.y1 = 0;

    // Find the raw bounding box
    font_string_flags flags = font_GIVEN_LENGTH | font_GIVEN_FONT | font_KERN
                              | font_GIVEN_BLOCK | font_RETURN_BBOX;
    if (matrix) flags |= font_GIVEN_TRFM;
    font_scan_string(*ref, text.c_str(), flags, INT_MAX, INT_MAX, &block,
                     matrix, text.length(), NULL, NULL, NULL, NULL);

    // Transform the resulting bounding box to internal units
    os_box box = transform_to_millipoint.inverse(block.bbox,
                                                 transform::round_out);
    box.x0 += left.x;
    box.y0 += left.y;
    box.x1 += left.x;
    box.y1 += left.y;

    // Return the bounding box
    return box;
}

/*
    Parameters  : text              - The text to process.
    Returns     : int               - The baseline width in internal units.
    Description : Calculate the baseline width for the specified text
                  without any additional spacing or transformations.
*/
int fontobj_handle::get_width(const string &text) const
{
    // Find the raw baseline width
    int width;
    font_string_flags flags = font_GIVEN_LENGTH | font_GIVEN_FONT | font_KERN;
    font_scan_string(*ref, text.c_str(), flags, INT_MAX, INT_MAX, NULL, NULL,
                     text.length(), NULL, &width, NULL, NULL);

    // Return the baseline width transformed to internal units
    return transform_to_millipoint.inverse(width);
}

/*
    Parameters  : background        - The background colour.
                  foreground        - The foreground colour.
    Returns     : os_error *        - Pointer to a corresponding error
                                      block, or NULL if no error.
    Description : Set the colours associated with this font handle.
*/
os_error *fontobj_handle::set_colour(os_colour background,
                                     os_colour foreground) const
{
    os_error *err = NULL;

    // Set the colours
    err = xcolourtrans_set_font_colours(*ref, background, foreground, 14,
                                        NULL, NULL, NULL);

    // Return any error produced
    return err;
}

/*
    Parameters  : trfm              - The transformation to apply.
                  text              - The text to process.
                  left              - The start position in internal units.
                  spacing           - Additional spacing between characters.
                  trfm              - Font transformation matrix.
    Returns     : os_error *        - Pointer to a corresponding error
                                      block, or NULL if no error.
    Description : Paint the specified text.
*/
os_error *fontobj_handle::paint(const transform_internal &trfm,
                                const string &text, const os_coord &left,
                                int spacing, const os_trfm *matrix) const
{
    os_error *err = NULL;

    // Calculate the start position in millipoint
    os_coord start = trfm.to_millipoint()(left);

    // Construct a paint block for the additional spacing
    font_paint_block block;
    block.space.x = trfm.to_millipoint()(spacing);
    block.space.y = 0;
    block.letter.x = trfm.to_millipoint()(spacing);
    block.letter.y = 0;
    block.rubout.x0 = block.rubout.y0 = block.rubout.x1 = block.rubout.y1 = 0;

    // Paint the text
    font_string_flags flags = font_GIVEN_LENGTH | font_GIVEN_FONT | font_KERN;
    if (spacing) flags |= font_GIVEN_BLOCK;
    if (matrix) flags |= font_GIVEN_TRFM;
    err = xfont_paint(*ref, text.c_str(), flags, start.x, start.y, &block,
                      matrix, text.length());

    // Return any error produced
    return err;
}

/*
    Parameters  : font      - The name of the font.
                  size      - The size of the font in internal units.
    Returns     : -
    Description : Constructor.
*/
fontobj_obj::fontobj_obj(const string &font, int size) : handle(font, size)
{
    // Store the font size
    this->size = size;

    // Justification not valid initially
    valid = FALSE;

    // Dummy values for everything else
    align = align_left;
    base.x0 = base.y0 = base.x1 = base.y1 = 0;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
fontobj_obj::~fontobj_obj()
{
    // No action required
}

/*
    Parameters  : text              - The text to justify.
    Returns     : void
    Description : Set the text.
*/
void fontobj_obj::set_text(const string &text)
{
    // Store the text
    this->text = text;

    // Invalidate the justification
    valid = FALSE;
}

/*
    Parameters  : pos               - The horizontal position to use for
                                      justification.
                  base              - The vertical position of the baseline.
    Returns     : void
    Description : Set the justification point.
*/
void fontobj_obj::set_position(int pos, int base)
{
    // Store the baseline position
    this->base.x0 = this->base.x1 = pos;
    this->base.y0 = this->base.y1 = base;

    // Invalidate the justification
    valid = FALSE;
}

/*
    Parameters  : left              - The left-hand edge of the baseline
                                      to use for justification.
                  right             - The right-hand edge of the baseline
                                      to use for justification.
                  base              - The vertical position of the baseline.
    Returns     : void
    Description : Set the justification line.
*/
void fontobj_obj::set_position(int left, int right, int base)
{
    // Store the baseline position
    this->base.x0 = left;
    this->base.x1 = right;
    this->base.y0 = this->base.y1 = base;

    // Invalidate the justification
    valid = FALSE;
}

/*
    Parameters  : align             - The required alignment.
    Returns     : void
    Description : Set the text alignment.
*/
void fontobj_obj::set_alignment(alignment_style align)
{
    // Store the aligment
    this->align = align;

    // Invalidate the justification
    valid = FALSE;
}

/*
    Paramters   : void
    Returns     : os_coord          - The text start position.
    Description : Obtain the baseline start position after justification.
*/
os_coord fontobj_obj::get_left() const
{
    // Calculate the justification
    justify();

    // Return the baseline start position
    os_coord left;
    left.x = justified.x0;
    left.y = justified.y0;
    return left;
}

/*
    Parameters  : void
    Returns     : int               - Additional spacing per character.
    Description : Obtain the additional inter-character spacing required
                  to justify the text.
*/
int fontobj_obj::get_spacing() const
{
    // Calculate the justification
    justify();

    // Return the extra spacing
    return spacing;
}

/*
    Parameters  : void
    Returns     : int               - Font width.
    Description : Obtain the modified font width required to jutify the
                  text if additional inter-character spacing is not used.
*/
int fontobj_obj::get_width() const
{
    // Calculate the justification
    justify();

    // Return the adjusted font width
    return width;
}

/*
    Parameters  : void
    Returns     : os_box            - The underline rectangle.
    Description : Obtain the rectangle to underline the text.
*/
os_box fontobj_obj::get_underline() const
{
    // Calculate the justification
    justify();

    // Get the recommended underline position
    int bottom;
    int top;
    handle.get_underline(bottom, top);

    // Return the underline rectangle
    os_box box;
    box.x0 = justified.x0;
    box.y0 = justified.y0 + bottom;
    box.x1 = justified.x1;
    box.y1 = justified.y1 + top;
    return box;
}

/*
    Parameters  : void
    Returns     : os_box            - The strikethrough rectangle.
    Description : Obtain the rectangle to strikethrough the text.
*/
os_box fontobj_obj::get_strikethrough() const
{
    // Calculate the justification
    justify();

    // Get the recommended strikethrough position
    int bottom;
    int top;
    handle.get_strikethrough(bottom, top);

    // Return the underline rectangle
    os_box box;
    box.x0 = justified.x0;
    box.y0 = justified.y0 + bottom;
    box.x1 = justified.x1;
    box.y1 = justified.y1 + top;
    return box;
}

/*
    Parameters  : void
    Returns     : void
    Description : Calculate the required justification.
*/
void fontobj_obj::justify() const
{
    // No action required if justification already calculated
    if (!valid)
    {
        // Remove const qualification to simulate mutable behaviour
        fontobj_obj *obj = (fontobj_obj *) this;

        // Get the unadjusted text width
        int raw = handle.get_width(text);
        int requested = base.x1 - base.x0;

        // Calculate the enhanced justification
        int actual;
        if ((int(requested * FONTOBJ_JUSTIFY_MIN + 0.5) < raw)
            && (raw < int(requested * FONTOBJ_JUSTIFY_MAX + 0.5))
            && (1 < text.length()))
        {
            // Use enhanced justification
            obj->spacing = transform_base::round(double(requested - raw)
                                                 / (text.length() - 1));
            obj->width = transform_base::round(size * double(requested) / raw);
            actual = requested;
        }
        else
        {
            // No enhanced justification
            obj->spacing = 0;
            obj->width = size;
            actual = raw;
        }

        // Apply the requested alignment
        obj->justified.y0 = base.y0;
        obj->justified.y1 = base.y1;
        switch (align)
        {
            case align_centre:
                // Align to centre of baseline
                obj->justified.x0 = (base.x0 + base.x1 - actual + 1) / 2;
                obj->justified.x1 = obj->justified.x0 + actual;
                break;

            case align_right:
                // Align to right of baseline
                obj->justified.x0 = base.x1 - actual;
                obj->justified.x1 = base.x1;
                break;

            case align_left:
            default:
                // Align to left of baseline
                obj->justified.x0 = base.x0;
                obj->justified.x1 = base.x0 + actual;
                break;
        }

        // Mark the justification as valid
        obj->valid = TRUE;
    }
}

/*
    Parameters  : face              - The font name.
                  bold              - Is a bold style required.
                  italic            - Is an italic style required.
                  font              - The name of the corresponding RISC OS
                                      font.
    Returns     : bool              - Was the mapping found.
    Description : Look for the specified font mapping.
*/
static bool fontobj_map(const string &face, bool bold, bool italic,
                        string &font)
{
    // Construct the appropriate tag
    string tag;
    if (bold && italic)
    {
        tag = tag_store::tag(fontobj_tag_font, face,
                             fontobj_tag_bold, fontobj_tag_italic);
    }
    else if (bold)
    {
        tag = tag_store::tag(fontobj_tag_font, face, fontobj_tag_bold);
    }
    else if (italic)
    {
        tag = tag_store::tag(fontobj_tag_font, face, fontobj_tag_italic);
    }
    else
    {
        tag = tag_store::tag(fontobj_tag_font, face);
    }

    // Lookup the requested mapping
    font = options_current.get_str(tag);

    // Return whether a mapping was found
    return !font.empty();
}

/*
    Parameters  : epoc              - The EPOC font name.
                  screen            - The screen font number.
                  bold              - Is a bold style required.
                  italic            - Is an italic style required.
    Returns     : string            - The name of the corresponding RISC OS
                                      font.
    Description : Map an EPOC font to the closest RISC OS equivalent.
*/
string fontobj_obj::map(const string &epoc, bits screen, bool bold, bool italic)
{
    // Try different styles of the requested font
    string font;
    if (!(bold && italic && fontobj_map(epoc, TRUE, TRUE, font))
        && !(bold && fontobj_map(epoc, TRUE, FALSE, font))
        && !(italic && fontobj_map(epoc, FALSE, TRUE, font))
        && !fontobj_map(epoc, FALSE, FALSE, font))
    {
        // Try different styles of the screen font
        char face[20];
        sprintf(face, FONTOBJ_SCREEN, screen);
        if (!(bold && italic && fontobj_map(face, TRUE, TRUE, font))
            && !(bold && fontobj_map(face, TRUE, FALSE, font))
            && !(italic && fontobj_map(face, FALSE, TRUE, font))
            && !fontobj_map(face, FALSE, FALSE, font))
        {
            // Try the default font
            font = options_current.get_str(fontobj_tag_font);
        }
    }

    // Return the result
    return font;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the font support.
*/
void fontobj_init()
{
    // Check whether font options exist
    if (!options_current.exist(fontobj_tag_font))
    {
        // Set the default mappings
        for (int i = 0; i < FONTOBJ_DEFAULT_MAP_SIZE; i++)
        {
            options_current.set_str(fontobj_default_map[i].tag,
                                    fontobj_default_map[i].value);
        }
    }
}
