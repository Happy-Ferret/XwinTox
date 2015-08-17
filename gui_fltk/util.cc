#include <cmath>

#include "util.h"
#include "misc.h"

ContactsEntry * FindContactEntry (XwinTox * Xw, unsigned int num)
{
    for (const auto entry : Xw->sidebar->contacts->entries)
    {
        if (entry->contact->wNum == num)
            return entry;
    }

    return 0;
}

GMessageArea * FindContactMArea (XwinTox * Xw, XWContact_t * contact)
{
    for (const auto messagearea : Xw->contents->messageareas)
    {
        if (messagearea->contact == contact)
            return messagearea;
    }

    return 0;
}

GMessageArea * FindContactMArea (XwinTox * Xw, unsigned int num)
{
    for (const auto messagearea : Xw->contents->messageareas)
    {
        if (messagearea->contact->wNum == num)
            return messagearea;
    }

    return 0;
}

XWContact_t * FindContact (XwinTox * Xw, unsigned int id)
{
    XWContact_t * c = 0;
    List_t * lstContacts = Xw->sidebar->contacts->lstContacts;

    LIST_ITERATE_OPEN (lstContacts)
    XWContact_t * cn = static_cast< XWContact_t * > (e_data);
    if (cn->wNum == id)
    {
        c = cn;
        break;
    }
    LIST_ITERATE_CLOSE (lstContacts)

    dbg ("C = %p\n", c);
    return c;
}

char * GetDisplayName (XWContact_t * contact, size_t LenLimit)
{
    static char name[255] = {0};

    if (contact->pszName == 0 || strlen (contact->pszName) == 0)
    {
        strncpy (name, contact->pszID, LenLimit);
        name[LenLimit - 2] = '.';
        name[LenLimit - 1] = '.';
        name[LenLimit] = '.';
        name[LenLimit + 1] = '\0';
    }
    else if (strlen (contact->pszName) >= LenLimit)
    {
        strncpy (name, contact->pszName, LenLimit);
        name[LenLimit - 2] = '.';
        name[LenLimit - 1] = '.';
        name[LenLimit] = '.';
        name[LenLimit + 1] = '\0';
    }
    else
    {
        strcpy (name, contact->pszName);
    }

    return name;
}

char * GetDisplayStatus (XWContact_t * contact, size_t LenLimit)
{
    static char status[255] = {0};

    if (contact->pszStatus == 0 || strlen (contact->pszStatus) == 0)
    {
        strcpy (status, "Unknown");
    }
    else if (strlen (contact->pszStatus) >= LenLimit)
    {
        strcpy (status, GetShortenedText (contact->pszStatus, LenLimit));
    }
    else if (strlen (contact->pszStatus) > 0)
    {
        strcpy (status, contact->pszStatus);
    }

    return status;
}

char * GetShortenedText (char * text, size_t LenLimit)
{
    static char name[255] = {0};

    strncpy (name, text, LenLimit);
    name[LenLimit - 2] = '.';
    name[LenLimit - 1] = '.';
    name[LenLimit] = '.';
    name[LenLimit + 1] = '\0';
    return name;
}

const char * GetDisplaySize (unsigned int bytes)
{
    static char dsize[255] = {0};
    const char * suffixes[5] = {"B", "KB", "MB", "GB", "TB"};
    double s = bytes;
    int o = 0, p = 2;

    while (s >= 1024 && o + 1 < bytes)
    {
        o++;
        s = s / 1024;
    }

    if (s - floor (s) == 0.0)
        p = 0;
    else if (s - floor (s) <= 0.1)
        p = 1;

    sprintf (dsize, "%.*f %s", p, s, suffixes[o]);
    return dsize;
}

void RemoveSpaces (char * s)
{
    char * p = s;
    int l = strlen (p);

    while (isspace (p[l - 1]))
        p[--l] = 0;

    while (*p && isspace (*p))
        ++p, --l;

    memmove (s, p, l + 1);
}
