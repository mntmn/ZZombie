/*

  (C) 2021 MNT Research GmbH (AmigaOS Backport and Cleanup)
  (C) 1995-2020 The AROS Development Team
  (C) 2002-2005 Harry Sintonen
  (C) 2005-2007 Pavel Fedin
  $Id$

  Desc: Assign CLI command
  Lang: English
*/

#include <proto/exec.h>
#include <proto/dos.h>

// TODO: IPTR not optimal yet
#define IPTR ULONG
#define BNULL (BPTR)0

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/exall.h>

#include <string.h>

#define DEBUG_ASSIGN(x)

/******************************************************************************


    NAME

        Assign [(name):] [{(target)}] [LIST] [EXISTS] [DISMOUNT] [DEFER]
               [PATH] [ADD] [PREPEND] [REMOVE] [VOLS] [DIRS] [DEVICES]

    SYNOPSIS

        NAME, TARGET/M, LIST/S, EXISTS/S, DISMOUNT/S, DEFER/S, PATH/S, ADD/S,
        PREPEND/S, REMOVE/S, VOLS/S, DIRS/S, DEVICES/S

    LOCATION

        C:

    FUNCTION

        ASSIGN creates a reference to a file or directory. The reference
        is a logical device name which makes it very convenient to specify
        assigned objects using the reference instead of their paths.

        If the NAME and TARGET arguments are given, ASSIGN assigns the
        given logical name to the specified target. If the NAME given is
        already assigned to a file or directory the new target replaces the
        previous target. A colon must be included after the NAME argument.

        If only the NAME argument is given, any assigns to that NAME are
        removed. If no arguments whatsoever are given, all logical
        assigns are listed.

    INPUTS

        NAME      --  the name that should be assigned to a file or directory
        TARGET    --  one or more files or directories to assign the NAME to
        LIST      --  list all assigns made
        EXISTS    --  display the target of the specified assign. If NAME is
                      not assigned, set the condition flag to WARN
        DISMOUNT  --  remove the volume or device NAME from the dos list
        DEFER     --  make an ASSIGN to a path or directory that not need to
                      exist at the time of assignment. The first time the
                      NAME is referenced the NAME is bound to the object
        PATH      --  path is assigned with a non-binding assign. This means
                      that the assign is re-evaluated each time a reference
                      to NAME is done. Like for DEFER, the path doesn't have
                      to exist when the ASSIGN command is executed
        ADD       --  don't replace an assign but add another object for a
                      NAME (multi-assigns)
        PREPEND   --  like ADD, but puts the assign at the front of the list
        REMOVE    --  remove an ASSIGN
        VOLS      --  show assigned volumes if in LIST mode
        DIRS      --  show assigned directories if in LIST mode
        DEVICES   --  show assigned devices if in LIST mode


    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

        The assign command has many switches. This together with the somewhat
        messy handling of DosList:s by dos.library makes the operation rather
        complicated.

        There are some fundamental building blocks that defines the semantics
        of the Assign command.

        Only one of the switches ADD, PREPEND, REMOVE, PATH and DEFER may be specified.

        If EXISTS is specified, only the name parameter is important.

        The implementation is split up in two fundamental procedures.

        doAssign()     --  make [a number of] assigns
        showAssigns()  --  show the available assigns
        checkAssign()  --  check if a particular assign exists

******************************************************************************/

//struct UtilityBase *UtilityBase;

#ifndef __AROS__
#define AROS_BSTR_strlen(s) *((UBYTE *)BADDR(s))
#endif

static const char version[] __attribute__((used)) = "\0$VER: Assign 50.13 (04.08.2021) © AROS" ;

/* Prototypes */

static int checkAssign(STRPTR name);
static int doAssign(STRPTR name, STRPTR *target, BOOL dismount, BOOL defer, BOOL path, BOOL add, BOOL prepend, BOOL remove);
static void showAssigns(BOOL vols, BOOL dirs, BOOL devices);
static int removeAssign(STRPTR name);
static STRPTR GetFullPath(BPTR lock);

#define DeferPrintf Printf
#define DeferPutStr Printf

static
const UBYTE argtemplate[] =
  "NAME,"
  "TARGET/M,"
  "LIST/S,"
  "EXISTS/S,"
  "DISMOUNT/S,"
  "DEFER/S,"
  "PATH/S,"
  "ADD=APPEND/S,"
  "PREPEND/S,"
  "REMOVE/S,"
  "VOLS/S,"
  "DIRS/S,"
  "DEVICES/S";

struct ArgList
{
  STRPTR name;
  STRPTR *target;
  IPTR list;
  IPTR exists;
  IPTR dismount;
  IPTR defer;
  IPTR path;
  IPTR add;
  IPTR prepend;
  IPTR remove;
  IPTR vols;
  IPTR dirs;
  IPTR devices;
};

struct ArgList arglist;

int main()
{
  struct RDArgs *readarg;
  struct ArgList arglist;
  struct ArgList *args = &arglist;
  int error = RETURN_OK;

  DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",37);

  if (DOSBase) {
    memset(&arglist, 0, sizeof(arglist));
    readarg = ReadArgs(argtemplate, (IPTR *)args, NULL);

    if (readarg) {
      /* Verify mutually exclusive args
       */
      if ((args->add!=0) + (args->prepend!=0) + (args->remove!=0) + (args->path!=0) + (args->defer!=0) > 1) {
        PutStr("Only one of ADD/APPEND, PREPEND, REMOVE, PATH or DEFER is allowed\n");
        FreeArgs(readarg);
        CloseLibrary((struct Library *) DOSBase);
        return RETURN_FAIL;
      }

      /* Check device name
       */
      if (args->name) {
        char *pos;

        /* Correct assign name construction? The rule is that the device name
         * should end with a colon at the same time as no other colon may be
         * in the name.
         */
        pos = strchr(args->name, ':');
        if (!pos || pos[1]) {
          Printf("Invalid device name %s\n", (IPTR)args->name);
          FreeArgs(readarg);
          CloseLibrary((struct Library *) DOSBase);
          return RETURN_FAIL;
        }
      }

      /* If the EXISTS keyword is specified, we only care about NAME */
      if (args->exists) {
        error = checkAssign( args->name);
      }
      else if (args->name) {
        /* If a NAME is specified, our primary task is to add or
           remove an assign */

        error = doAssign(args->name, args->target, args->dismount, args->defer,
                         args->path, args->add, args->prepend, args->remove);
        if (args->list) {
          /* With the LIST keyword, the current assigns will be
             displayed also when (after) making an assign */

          showAssigns(args->vols, args->dirs, args->devices);
        }
      }
      else {
        /* If no NAME was given, we just show the current assigns
           as specified by the user (VOLS, DIRS, DEVICES) */

        showAssigns(args->vols, args->dirs, args->devices);
      }
      FreeArgs(readarg);
    }
    CloseLibrary((struct Library *) DOSBase);
  }

  return error;
}

static
void showAssigns(BOOL vols, BOOL dirs, BOOL devices)
{
  ULONG           lockBits = LDF_READ;
  struct DosList *dl;

  /* If none of the parameters are specified, everything should be
     displayed */
  if (!(vols || dirs || devices))
    {
      vols    = TRUE;
      dirs    = TRUE;
      devices = TRUE;
    }

  lockBits |= vols    ? LDF_VOLUMES : 0;
  lockBits |= dirs    ? LDF_ASSIGNS : 0;
  lockBits |= devices ? LDF_DEVICES : 0;

  dl = LockDosList(lockBits);

  /* FIXME: GetFullPath() breaks LockDosList()'s Forbid()! */
  /* Note: This should be ok as long as we don't have ks 1.x compatibility.
   */

  if (vols)
    {
      struct DosList *tdl = dl;

      DeferPutStr("Volumes:\n");

      /* Print all mounted volumes */
      while ((tdl = NextDosEntry(tdl, LDF_VOLUMES)))
        {
          DeferPrintf("%b [Mounted]\n", tdl->dol_Name);
        }
    }

  if (dirs)
    {
      struct DosList *tdl = dl;
      int             count;

      DeferPutStr("\nDirectories:\n");

      /* Print all assigned directories */
      while ((tdl = NextDosEntry(tdl, LDF_ASSIGNS)))
        {
          DeferPrintf("%b ", tdl->dol_Name);

          for (count = 14 - AROS_BSTR_strlen(tdl->dol_Name); count > 0; count--)
            {
              DeferPutStr(" ");
            }

          switch (tdl->dol_Type)
            {
            case DLT_LATE:
              DeferPrintf("<%s>\n", (IPTR)tdl->dol_misc.dol_assign.dol_AssignName);
              break;

            case DLT_NONBINDING:
              DeferPrintf("[%s]\n", (IPTR)tdl->dol_misc.dol_assign.dol_AssignName);
              break;

            default:
              {
                STRPTR             dirName;     /* For NameFromLock() */
                struct AssignList *nextAssign;  /* For multiassigns */

                dirName = GetFullPath( tdl->dol_Lock);

                if (dirName)
                  {
                    DeferPutStr(dirName);
                    FreeVec(dirName);
                  }
                DeferPutStr("\n");

                nextAssign = tdl->dol_misc.dol_assign.dol_List;

                while (nextAssign)
                  {
                    dirName = GetFullPath( nextAssign->al_Lock);

                    if (dirName)
                      {
                        DeferPrintf("             + %s\n", (IPTR)dirName);
                        FreeVec(dirName);
                      }

                    nextAssign = nextAssign->al_Next;
                  }
              }

              break;
            }
        } /* while (NextDosEntry()) */
    }

  if (devices)
    {
      struct DosList *tdl = dl;
      int             count = 0; /* Used to make sure that as most 5 entries are printed per line */

      DeferPutStr("\nDevices:\n");

      /* Print all assigned devices */
      while ((tdl = NextDosEntry(tdl, LDF_DEVICES)))
        {
          DeferPrintf("%b%lc", tdl->dol_Name, ++count % 5 ? ' ' : '\n');
        }

      if (count % 5)
        {
          DeferPutStr("\n");
        }
    }

  UnLockDosList(lockBits);
}


static
STRPTR GetFullPath(BPTR lock)
{
  STRPTR buf;       /* Pointer to the memory allocated for the string */
  ULONG  size;      /* Holder of the (growing) size of the string */

  for (size = 512; ; size += 512)
    {
      buf = AllocVec(size, MEMF_ANY);
      if (!buf)
        {
          break;
        }

      if (NameFromLock(lock, buf, size))
        {
          return buf;
        }

      FreeVec(buf);

      if (IoErr() != ERROR_LINE_TOO_LONG)
        {
          break;
        }
    }

  return NULL;
}


static
int doAssign(STRPTR name, STRPTR *target, BOOL dismount, BOOL defer, BOOL path,
             BOOL add, BOOL prepend, BOOL remove)
{
  STRPTR colon;
  BPTR   lock = BNULL;
  int    i;

  int  error = RETURN_OK;
  LONG ioerr = 0;
  BOOL cancel = FALSE;
  BOOL success = TRUE;

  if (dismount)
    {
      struct MsgPort *dp;
      struct Process *tp;

      tp=(struct Process *)FindTask(NULL);
      tp->pr_WindowPtr = (APTR)-1;
      dp = DeviceProc(name);
      DEBUG_ASSIGN(Printf("doassign: dp <%08X>\n",dp));
      if (dp)
        {
          success = DoPkt(dp,ACTION_DIE,0,0,0,0,0);
          ioerr = IoErr();
          DEBUG_ASSIGN(Printf("doassign: ACTION_DIE returned %ld\n",success));
        }
    }

  colon = strchr(name, ':');

  *colon = '\0';        /* Remove trailing colon; name[] is changed! */

  DEBUG_ASSIGN(Printf("doassign: name <%s>\n", name));

  /* This is a little bit messy... We first remove the 'name' assign
   * and later in the loop the target assigns.
   */

  if (dismount)
    {
      if ((!success) && (ioerr == ERROR_ACTION_NOT_KNOWN))
        {
          struct DosList *dl;
          struct DosList *fdl;

          DEBUG_ASSIGN(PutStr("Removing device node\n"));
          dl = LockDosList(LDF_VOLUMES | LDF_DEVICES | LDF_WRITE);

          fdl = FindDosEntry(dl, name, LDF_VOLUMES | LDF_DEVICES);

          /* Note the ! for conversion to boolean value */
          if (fdl)
            {
              success = RemDosEntry(fdl);
              if (success)
                FreeDosEntry(fdl);
              else
                ioerr = ERROR_OBJECT_IN_USE;
            } else
            ioerr = ERROR_OBJECT_NOT_FOUND;

          UnLockDosList(LDF_VOLUMES | LDF_DEVICES | LDF_WRITE);
        }
    }
  else
    {
      if (target == NULL || *target == NULL)
        {
          error = removeAssign( name);
          if (error)
            {
              ioerr = IoErr();
              cancel = TRUE;
            }
        }
    }

  /* AmigaDOS doesn't use RETURN_WARN here... but it should? */
  error = success ? error : RETURN_WARN;
  DEBUG_ASSIGN(Printf("error: %d\n", error));

  // The Loop over multiple targets starts here

  if (target)
    {
      for (i = 0; target[i]; i++)
        {
          cancel = FALSE;

          DEBUG_ASSIGN(Printf("doassign: target <%s>\n", target[i]));
          if (!(path || defer || dismount))
            {
              lock = Lock(target[i], SHARED_LOCK);

              if (!lock)
                {
                  Printf("Can't find %s\n", (IPTR)target[i]);
                  return RETURN_FAIL;
                }
            }

          if (remove)
            {
              if (!RemAssignList(name, lock))
                {
                  Printf("Can't subtract %s from %s\n", (IPTR)target[i], (IPTR)name);
                  error = RETURN_FAIL;
                }
              UnLock(lock);
            }
          else if (path)
            {
              if (!AssignPath(name, target[i]))
                {
                  ioerr = IoErr();
                  error = RETURN_FAIL;
                  DEBUG_ASSIGN(Printf("doassign AssignPath error %ld\n",error));
                }
            }
          else if ((add) || (prepend))
            {
              BOOL success = FALSE;
              //if (add)
              success = AssignAdd(name, lock);
              //else
              //success = AssignAddToList(name, lock, 0);

              if (!success)
                {
                  struct DosList *dl;

                  error = RETURN_FAIL;
                  ioerr = IoErr();
                  DEBUG_ASSIGN(Printf("doassign add/prepend error %ld\n",error));

                  /* Check if the assign doesn't exist at all. If so, create it.
                   * This fix bug id 145. - Piru
                   */
                  dl = LockDosList(LDF_ASSIGNS | LDF_READ);
                  dl = FindDosEntry(dl, name, LDF_ASSIGNS);
                  UnLockDosList(LDF_ASSIGNS | LDF_READ);

                  if (!dl)
                    {
                      if (AssignLock(name, lock))
                        {
                          error = RETURN_OK;
                          lock = BNULL;
                        }
                      else
                        {
                          ioerr = IoErr();
                          DEBUG_ASSIGN(Printf("doassign AssignLock error %ld\n", error));
                        }
                    }

                  if (lock)
                    UnLock(lock);

                  if (error && ioerr != ERROR_OBJECT_EXISTS)
                    {
                      Printf("Can't add %s to %s\n", (IPTR)target[i], (IPTR)name);
                    }
                }
            }
          else if (defer)
            {
              if (!AssignLate(name, target[i]))
                {
                  ioerr = IoErr();
                  UnLock(lock);
                  error = RETURN_FAIL;
                  DEBUG_ASSIGN(Printf("doassign AssignLate error %ld\n",error));
                }
            }
          else
            {
              /* If no extra parameters are specified we just do a regular
                 assign (replacing any possible previous assign with that
                 name. The case of target being NULL is taken care of above.
              */
              if (!AssignLock(name, lock))
                {
                  ioerr = IoErr();
                  cancel = TRUE;
                  UnLock(lock);
                  error = RETURN_FAIL;
                  DEBUG_ASSIGN(Printf("doassign AssignLock error %ld\n",error));
                }
              /* If there are several targets, the next ones have to be added. */
              add = TRUE;
            }

          /* We break as soon as we get a serious error */
          if (error >= RETURN_FAIL)
            {
              break;
            }

        } /* loop through all targets */
    }

  if (error)
    {
      if (ioerr == ERROR_OBJECT_EXISTS)
        {
          Printf("Can't %s %s\n", (IPTR)(cancel ? "cancel" : "assign"), (IPTR)name);
        }
      else
        {
          PrintFault(ioerr, NULL);
        }
    }

  return error;
}


static
int removeAssign(STRPTR name)
{
  /* In case no target is given, the 'name' assign should be removed.
   * The AmigaDOS semantics for this is apparently that the error
   * code is never set even if the assign didn't exist.
   */

  if (!AssignLock(name, BNULL))
    {
      return RETURN_FAIL;
    }
  return RETURN_OK;
}


static
int checkAssign(STRPTR name)
{
  STRPTR colon;
  struct DosList *dl;
  int             error = RETURN_OK;

  if (!name)
    name = "";

  colon = strchr(name, ':');
  if (colon)
    {
      *colon = '\0';
    }

  dl = LockDosList(LDF_DEVICES | LDF_ASSIGNS | LDF_VOLUMES | LDF_READ);

  /* FIXME: GetFullPath() breaks LockDosList()'s Forbid()! */
  /* Note: This should be ok as long as we don't have ks 1.x compatibility.
   */

  dl = FindDosEntry(dl, name, LDF_DEVICES | LDF_ASSIGNS | LDF_VOLUMES);
  if (dl)
    {
      struct DosList *tdl = dl;
      int             count;

      switch (dl->dol_Type)
        {
        case DLT_DEVICE:
          DeferPrintf("%b\n", tdl->dol_Name);
          break;

        case DLT_VOLUME:
          DeferPrintf("%b [Mounted]\n", tdl->dol_Name);
          break;

        case DLT_DIRECTORY:
        case DLT_LATE:
        case DLT_NONBINDING:

          DeferPrintf("%b ", tdl->dol_Name);

          for (count = 14 - *((UBYTE*)BADDR(tdl->dol_Name)); count > 0; count--)
            {
              DeferPutStr(" ");
            }

          switch (tdl->dol_Type)
            {
            case DLT_LATE:
              DeferPrintf("<%s>\n", (IPTR)tdl->dol_misc.dol_assign.dol_AssignName);
              break;

            case DLT_NONBINDING:
              DeferPrintf("[%s]\n", (IPTR)tdl->dol_misc.dol_assign.dol_AssignName);
              break;

            default:
              {
                STRPTR             dirName;     /* For NameFromLock() */
                struct AssignList *nextAssign;  /* For multiassigns */

                dirName = GetFullPath( tdl->dol_Lock);

                if (dirName)
                  {
                    DeferPutStr(dirName);
                    FreeVec(dirName);
                  }
                DeferPutStr("\n");

                nextAssign = tdl->dol_misc.dol_assign.dol_List;

                while (nextAssign)
                  {
                    dirName = GetFullPath(nextAssign->al_Lock);

                    if (dirName)
                      {
                        DeferPrintf("             + %s\n", (IPTR)dirName);
                        FreeVec(dirName);
                      }

                    nextAssign = nextAssign->al_Next;
                  }
              }
            }

          break;
        }
    }
  else
    {
      DeferPrintf("%s: not assigned\n", (IPTR)name);

      error = RETURN_WARN;
    }

  UnLockDosList(LDF_DEVICES | LDF_ASSIGNS | LDF_VOLUMES | LDF_READ);

  if (colon)
    *colon = ':';

  return error;
}
