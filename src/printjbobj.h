/*
    File        : printjbobj.h
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

// Only include header file once
#ifndef PRINTJBOBJ_H
#define PRINTJBOBJ_H

// Include cathlibcpp header files
#include "vector.h"

// Include project header files
#include "printpgobj.h"
#include "psifs.h"

// Print job object class
class printjbobj_obj
{
    // Reference counted details
    struct ref_class
    {
        vector<printpgobj_obj> pages;   // List of pages
        psifs_print_job_handle handle;  // Job handle (while active)
        psifs_print_job_status status;  // Status of the print job
        int count;                      // Number of references

        /*
            Parameters  : void
            Returns     : -
            Description : Constructor.
        */
        ref_class() : count(1) {}

        /*
            Parameters  : void
            Returns     : -
            Description : Destructor.
        */
        ~ref_class();

        /*
            Parameters  : void
            Returns     : void
            Description : Increment the reference count.
        */
        void inc();

        /*
            Parameters  : void
            Returns     : void
            Description : Decrement the reference count.
        */
        void dec();
    };
    ref_class *ref;

public:

    /*
        Parameters  : handle            - Handle of the print job.
        Returns     : -
        Description : Constructor.
    */
    printjbobj_obj(psifs_print_job_handle handle);

    /*
        Parameters  : pages             - The page to include in this job.
        Returns     : -
        Description : Constructor.
    */
    printjbobj_obj(const printpgobj_obj &page);

    /*
        Parameters  : pages             - The pages to include in this job.
        Returns     : -
        Description : Constructor.
    */
    printjbobj_obj(const vector<printpgobj_obj> &pages);

    /*
        Parameters  : job               - The print job to copy.
        Returns     : -
        Description : Constructor.
    */
    printjbobj_obj(const printjbobj_obj &job);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printjbobj_obj();

    /*
        Parameters  : job               - The print job to copy.
        Returns     : printjbobj_obj    - A reference to this job.
        Description : Page assignment.
    */
    printjbobj_obj &operator=(const printjbobj_obj &job);

    /*
        Parameters  : void
        Returns     : void
        Description : Update the status of this print job.
    */
    void update();

    /*
        Parameters  : void
        Returns     : bits              - The number of pages in the print job.
        Description : Return the number of pages in the print job.
    */
    bits pages() const;

    /*
        Parameters  : void
        Returns     : bool              - Is the print job complete.
        Description : Check whether all the pages of this print job have been
                      received.
    */
    bool complete() const;

    /*
        Parameters  : void
        Returns     : bool              - Has the print job been cancelled.
        Description : Check whether the print job has been cancelled.
    */
    bool cancelled() const;

    /*
        Parameters  : rend              - The object to perform the rendering.
        Returns     : void
        Description : Render the print job using the specified object.
    */
    void render(printrend_base &rend);

    /*
        Parameters  : page              - The required page number (starting
                                          from 1 for the first page).
        Returns     : printpgobj_obj    - The requested page.
        Description : A single page from the print job.
    */
    printpgobj_obj operator[](size_t page) const;

    /*
        Parameters  : void
        Returns     : void
        Description : Update any print jobs being received.
    */
    static void update_all();
};

#endif
