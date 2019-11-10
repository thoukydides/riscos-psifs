/*
    File        : printjbobj.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job management for the PsiFS filer.

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
#include "printjbobj.h"

// Include oslib header files
#include "oslib/osfscontrol.h"

// Include cathlibcpp header files
#include "list.h"

// Include project header files
#include "scrap.h"

// A list of print job objects
static list<printjbobj_obj *> printjbobj_list;

/*
    Parameters  : void
    Returns     : -
    Description : Destructor.
*/
printjbobj_obj::ref_class::~ref_class()
{
    // Cancel the print job if the reference count has reached zero
    if (!count && (handle != psifs_PRINT_JOB_INVALID))
    {
        xpsifs_print_job_cancel(handle);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Increment the reference count.
*/
void printjbobj_obj::ref_class::inc()
{
    // Just increment the reference count
    count++;
}

/*
    Parameters  : void
    Returns     : void
    Description : Decrement the reference count.
*/
void printjbobj_obj::ref_class::dec()
{
    // Decrease the reference count and delete if appropriate
    if (!--count) delete this;
}

/*
    Parameters  : handle            - Handle of the print job.
    Returns     : -
    Description : Constructor.
*/
printjbobj_obj::printjbobj_obj(psifs_print_job_handle handle)
{
    // Create the print job
    ref = new ref_class;
    ref->handle = handle;
    ref->status = psifs_PRINT_JOB_RECEIVING;

    // Swallow the dummy first page
    psifs_print_job_data(handle, NULL);

    // Add to the list of print jobs
    printjbobj_list.push_back((printjbobj_obj *)(this));
}

/*
    Parameters  : pages             - The page to include in this job.
    Returns     : -
    Description : Constructor.
*/
printjbobj_obj::printjbobj_obj(const printpgobj_obj &page)
{
    // Create the print job
    ref = new ref_class;

    // Store the page
    ref->pages.push_back(page);
    ref->handle = psifs_PRINT_JOB_INVALID;
    ref->status = psifs_PRINT_JOB_COMPLETE;

    // Add to the list of print jobs
    printjbobj_list.push_back((printjbobj_obj *)(this));
}

/*
    Parameters  : pages             - The pages to include in this job.
    Returns     : -
    Description : Constructor.
*/
printjbobj_obj::printjbobj_obj(const vector<printpgobj_obj> &pages)
{
    // Create the print job
    ref = new ref_class;

    // Store the pages
    ref->pages = pages;
    ref->handle = psifs_PRINT_JOB_INVALID;
    ref->status = psifs_PRINT_JOB_COMPLETE;

    // Add to the list of print jobs
    printjbobj_list.push_back((printjbobj_obj *)(this));
}

/*
    Parameters  : job               - The print job to copy.
    Returns     : -
    Description : Constructor.
*/
printjbobj_obj::printjbobj_obj(const printjbobj_obj &job)
{
    // Increase the reference count of the print job being copied
    if (job.ref) job.ref->inc();

    // Copy the pointer to the reference counted details
    ref = job.ref;

    // Add to the list of print jobs
    printjbobj_list.push_back((printjbobj_obj *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printjbobj_obj::~printjbobj_obj()
{
    // Remove from the list of print jobs
    printjbobj_list.remove((printjbobj_obj *)(this));

    // Decrement the reference count
    if (ref) ref->dec();
}

/*
    Parameters  : job               - The print job to copy.
    Returns     : printjbobj_obj    - A reference to this job.
    Description : Page assignment.
*/
printjbobj_obj &printjbobj_obj::operator=(const printjbobj_obj &job)
{
    // Increase the reference count of the print job being copied
    if (job.ref) job.ref->inc();

    // Reduce the reference count of this print job
    if (ref) ref->dec();

    // Copy the pointer to the reference counted details
    ref = job.ref;

    // Return a pointer to this print job
    return *this;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the status of this print job.
*/
void printjbobj_obj::update()
{
    // No action unless active
    if (ref && (ref->handle != psifs_PRINT_JOB_INVALID))
    {
        bits received;
        bits read;

        // Poll the status of the print job
        ref->status = psifs_print_job_poll(ref->handle, NULL, &received, &read);

        // Read any new pages of print data
        while (read < received)
        {
            // Read the next page of data
            string temp(scrap_name());
            psifs_print_job_data(ref->handle, temp.c_str());

            // Add the page to the print job
            ref->pages.push_back(printpgobj_obj(temp));

            // Delete the temporary file
            xosfscontrol_wipe(temp.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);

            // Increment the number of pages read
            read++;
        }

        // Handle the end of the print job
        if (ref->status != psifs_PRINT_JOB_RECEIVING)
        {
            ref->handle = psifs_PRINT_JOB_INVALID;
        }
    }
}

/*
    Parameters  : void
    Returns     : bits              - The number of pages in the print job.
    Description : Return the number of pages in the print job.
*/
bits printjbobj_obj::pages() const
{
    return ref ? ref->pages.size() : 0;
}

/*
    Parameters  : void
    Returns     : bool              - Is the print job complete.
    Description : Check whether all the pages of this print job have been
                  received.
*/
bool printjbobj_obj::complete() const
{
    return !ref || (ref->status == psifs_PRINT_JOB_COMPLETE);
}

/*
    Parameters  : void
    Returns     : bool              - Has the print job been cancelled.
    Description : Check whether the print job has been cancelled.
*/
bool printjbobj_obj::cancelled() const
{
    return !ref || ((ref->handle == psifs_PRINT_JOB_INVALID)
                    && (ref->status != psifs_PRINT_JOB_COMPLETE));
}

/*
    Parameters  : rend              - The object to perform the rendering.
    Returns     : void
    Description : Render the print job using the specified object.
*/
void printjbobj_obj::render(printrend_base &rend)
{
    // No action unless active
    if (ref)
    {
        printpgobj_obj *i;

        // Render all of the pages
        for (i = ref->pages.begin(); rend && (i != ref->pages.end()); i++)
        {
            (*i).render(rend);
        }
    }
}

/*
    Parameters  : page              - The required page number (starting
                                      from 1 for the first page).
    Returns     : printpgobj_obj    - The requested page.
    Description : A single page from the print job.
*/
printpgobj_obj printjbobj_obj::operator[](size_t page) const
{
    return ref && (0 < page) && (page <= ref->pages.size())
           ? ref->pages[page - 1]
           : printpgobj_obj();
}

/*
    Parameters  : void
    Returns     : void
    Description : Update any print jobs being received.
*/
void printjbobj_obj::update_all()
{
    list_iterator<printjbobj_obj *> i;

    // Update all of the print jobs
    for (i = printjbobj_list.begin(); i != printjbobj_list.end(); i++)
    {
        (*i)->update();
    }
}
