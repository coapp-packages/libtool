/* loader-load_add_on.c --  dynamic linking for BeOS
   Copyright (C) 1998, 1999, 2000, 2004 Free Software Foundation, Inc.
   Originally by Thomas Tanner <tanner@ffii.org>

   NOTE: The canonical source of this file is maintained with the
   GNU Libtool package.  Report bugs to bug-libtool@gnu.org.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

As a special exception to the GNU Lesser General Public License,
if you distribute this file as part of a program or library that
is built using GNU libtool, you may include it under the same
distribution terms that you use for the rest of that program.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307  USA

*/

#include "lt__private.h"
#include "lt_dlloader.h"

/* Use the preprocessor to rename non-static symbols to avoid namespace
   collisions when the loader code is statically linked into libltdl.
   Use the "<module_name>_LTX_" prefix so that the symbol addresses can
   be fetched from the preloaded symbol list by lt_dlsym():  */
#define get_vtable	load_add_on_LTX_get_vtable

extern lt_user_dlloader *get_vtable (lt_user_data loader_data);


/* Boilerplate code to set up the vtable for hooking this loader into
   libltdl's loader list:  */
static lt_module vm_open  (lt_user_data loader_data, const char *filename);
static int	 vm_close (lt_user_data loader_data, lt_module module);
static void *	 vm_sym   (lt_user_data loader_data, lt_module module,
			  const char *symbolname);

/* Return the vtable for this loader, only the name and sym_prefix
   attributes (plus the virtual function implementations, obviously)
   change between loaders.  */
lt_user_dlloader *
get_vtable (lt_user_data loader_data)
{
  static lt_user_dlloader *vtable = 0;

  if (!vtable)
    {
      vtable = lt__zalloc (sizeof *vtable);
    }

  if (!vtable->name)
    {
      vtable->name		= "lt_load_add_on";
      vtable->module_open	= vm_open;
      vtable->module_close	= vm_close;
      vtable->find_sym		= vm_sym;
      vtable->dlloader_data	= loader_data;
      vtable->priority		= LT_DLLOADER_APPEND;
    }

  if (vtable->dlloader_data != loader_data)
    {
      LT__SETERROR (INIT_LOADER);
      return 0;
    }

  return vtable;
}



/* --- IMPLEMENTATION --- */


#include <kernel/image.h>

/* A function called through the vtable to open a module with this
   loader.  Returns an opaque representation of the newly opened
   module for processing with this loader's other vtable functions.  */
static lt_module
vm_open (lt_user_data loader_data, const char *filename)
{
  image_id image = 0;

  if (filename)
    {
      image = load_add_on (filename);
    }
  else
    {
      image_info info;
      int32 cookie = 0;
      if (get_next_image_info (0, &cookie, &info) == B_OK)
	image = load_add_on (info.name);
    }

  if (image <= 0)
    {
      LT__SETERROR (CANNOT_OPEN);
      image = 0;
    }

  return (lt_module) image;
}


/* A function called through the vtable when a particular module
   should be unloaded.  */
static int
vm_close (lt_user_data loader_data, lt_module module)
{
  int errors = 0;

  if (unload_add_on ((image_id) module) != B_OK)
    {
      LT__SETERROR (CANNOT_CLOSE);
      ++errors;
    }

  return errors;
}


/* A function called through the vtable to get the address of
   a symbol loaded from a particular module.  */
static void *
vm_sym (lt_user_data loader_data, lt_module module, const char *name)
{
  void *address = 0;
  image_id image = (image_id) module;

  if (get_image_symbol (image, name, B_SYMBOL_TYPE_ANY, address) != B_OK)
    {
      LT__SETERROR (SYMBOL_NOT_FOUND);
      address = 0;
    }

  return address;
}