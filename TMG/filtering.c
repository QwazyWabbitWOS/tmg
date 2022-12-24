
#include "g_local.h"
#include "filtering.h"

#define TEXT_FILTER_FILE		"textfilter.cfg"
#define DIM(a) ( sizeof(a) / sizeof(a[0]) )

//RAV
	char	*apszTextFilterStrings[1000];
	size_t		nTextFilterCount;

	char	*apszTextNonFilterStrings[1000];
	size_t		nTextNonFilterCount;
//

/**************************************************************************/

/**
 A wrapper for strncpy that unlike strncpy, always terminates strings with NUL.
 */
static void Strcpyn(char *pszDest, const char *pszSrc, size_t nDestSize)
{
	strncpy( pszDest, pszSrc, nDestSize - 1);
	pszDest[nDestSize-1] = '\0';
}

/**************************************************************************/

/**
 Case insensitive strstr
 */
static char *strstri(char *str1, char *str2)
{
	char *cp = (char *) str1;
	char *s1, *s2;

	if ( !*str2 )
		return((char *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char *) str2;

		while ( *s1 && *s2 && !( tolower(*s1) - tolower(*s2) ) ) {
			s1++; s2++;
		}

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

/**************************************************************************/

/** 
 Search for pszSubstring inside pszString, but consider identical
 characters between letters in pszSubstring to be a match.
*/
static char *StrStrSpanNoCase(char *pszString, char *pszSubstring)
{
	char *s1, *s2;

	// Is the substring just an empty string?  Then we have a match.
	if ( !*pszSubstring )
		return (char *)pszString;

	// Iterate over the target string and see if we can match the substring.
	for ( ; *pszString; pszString++)
	{
		s1 = pszString;
		s2 = (char *) pszSubstring;

		// Iterate and match the substring with the current position within
		// the target string.
		while ( *s1 && *s2 && !( tolower(*s1) - tolower(*s2) ) )
		{
			s1++;

			// Advance s1 again because we want to try to match every
			// other character in the substring instead of every sequential
			// character.
			if ( *s1)
				s1++;

			s2++;
		}

		// If we reached the end of the substring, then we must have had a
		// match through the full substring.
		if (!*s2)
		{
			// If every other character in the target string is the same
			// (a space for example), then we have a match.
			qboolean bOkay = true;
			char c;
			size_t nCount = strlen( pszSubstring) - 1;

			s1 = pszString + 1;

			// All in-between characters must match this.
			c = *s1;

			while ( nCount > 0)
			{
				if ( *s1 != c)
				{
					bOkay = false;
					break;
				}

				nCount--;
				s1 += 2;
			}

			if ( bOkay)
   			return pszString;
		}
	}

	return NULL;
}

/**************************************************************************/

/** 
 Does an fgets() but strips any trailing whitespace.
 */
static char *fgets_endws(char *pszBuffer, int nSize, FILE *pStream)
{
	char *psz;

	char *pszReturn = fgets( pszBuffer, nSize, pStream);

	if ( pszReturn == NULL)
		return NULL; // error or EOF

	// Iterate through the buffer and strip EOL characters.
	for ( psz = pszBuffer + strlen( pszBuffer); psz != pszBuffer; psz--)
	{
		if ( *psz == '\r' || *psz == '\n' || 
			 *psz == ' '  || *psz == '\t' || 
			 *psz == '\0')
		{
			*psz = '\0';
		}
		else
			break;
	}

	return pszReturn;
}


/*
 Replaces bad strings in the text with an asterisk.
 */
qboolean FilterText(char *pszText)
{
	size_t 	 i;
	char	 szBuffer[500];
	char	*p;
	char	*pSrc;
	qboolean bMatch = false;
	qboolean bSequence;

	Strcpyn( szBuffer, pszText, sizeof( szBuffer) );

	// Search for strings that we should NOT replace and set the high
	// bit on the characters so they won't be matched in further searches.

	// Don't automatically increment at the end of the for() loop, otherwise
	// multiple occurrences of a phrase will not be caught.
	for ( i = 0; i < nTextNonFilterCount; )
	{
		size_t nMatchLen;
		
		p = strstri( szBuffer, apszTextNonFilterStrings[i] );

		if ( p == NULL)
		{
			i++;
			continue;
		}
		
		// Got a match, so set the high bit on the characters.
		for ( nMatchLen = strlen( apszTextNonFilterStrings[i] );
				nMatchLen > 0;
				nMatchLen--)
		{
			*p += 128;
			p++;
		}

		// Don't increment i here, so we'll check for multiple occurrences.
	}

	// Now search for strings that are to be filtered out.

	// Don't automatically increment at the end of the for() loop, otherwise
	// multiple occurrences of a phrase will not be caught.
	for ( i = 0; i < nTextFilterCount; )
	{
		size_t nMatchLen;
		
		p = strstri( szBuffer, apszTextFilterStrings[i] );
		
		// Got a match?
		if ( p != NULL)
		{
			// Got a match, so replace all of the matched characters with a
			// chr(1)
			for ( nMatchLen = strlen( apszTextFilterStrings[i] );
					nMatchLen > 0;
					nMatchLen--)
			{
				*p = 1;
				p++;
			}

			// Don't increment i here, so we'll check for multiple occurrences.
			bMatch = true;
			continue;
		}

		// Check for skip character matches, but don't check if the filter
		// string is only two characters or less.
		if ( strlen( apszTextFilterStrings[i] ) > 2
			&& NULL != (p = StrStrSpanNoCase( szBuffer, apszTextFilterStrings[i])) )
		{
			// Replace matched characters with a chr(1)
			for ( nMatchLen = strlen( apszTextFilterStrings[i] );
					nMatchLen > 0;
					nMatchLen--)
			{
				*p = 1;
				p++;

				// We can set the next character because we assume that
				// the nMatchLen is greater than 1 (because single match would
				// have been caught in the strstri check).
				// For a substring length of two or more characters, there must
				// be at least one dummy character in the middle.
				if ( nMatchLen > 1)
				{
					*p = 1;
					p++;
				}
			}

			// Don't increment i here, so we'll check for multiple occurrences.
			bMatch = true;
			continue;
		}

		// No match, so go to the next filter string one.
		i++;
	}

	if ( !bMatch)
		return false;

	// Replace multiple occurrences of the chr(1) with a single asterisk by
	// copying back into pszText.
	pSrc = pszText;
	bSequence = false;
	for ( p = szBuffer; *p != '\0'; p++)
	{
		if ( *p != 1)
		{
			*pSrc = *p;
			bSequence = false;
			pSrc++;
			continue;
		}

		// We're in the middle of a filter sequence, so skip to the next
		// character.
		if ( bSequence)
			continue;

		bSequence = true;
		*pSrc = '*';
		pSrc++;
	}

	// Terminate the source string here because we may have taken characters
	// out of the string.
	*pSrc = '\0';

	// Now convert all of the characters that had the high bit set back
	// to regular characters.
	for ( p = pszText; *p != '\0'; p++)
	{
		unsigned char c = *p;
		if ( c > 128)
			*p = c - 128;
	}

	return true;
}

/**************************************************************************/

static void PurgeTextFilterInfo(void)
{
	size_t i;

	for ( i = 0; i < nTextFilterCount; i++)
	{
		gi.TagFree( apszTextFilterStrings[i] );
		apszTextFilterStrings[i] = NULL;
	}

	nTextFilterCount = 0;

	for ( i = 0; i < nTextNonFilterCount; i++)
	{
		gi.TagFree( apszTextNonFilterStrings[i] );
		apszTextNonFilterStrings[i] = NULL;
	}

	nTextNonFilterCount = 0;
}

/**************************************************************************/

/**
 Comparison function for use with qsort function
 */
static int CompareStringLength(void const *a, void const *b)
{
	size_t n1, n2;

	n1 = strlen( *(char **) a);
	n2 = strlen( *(char **) b);

	// Reverse comparison so strings will be sorted in decreasing order.
	return (int) (n2 - n1);
}

/**************************************************************************/

void LoadTextFilterInfo(void)
{
	FILE	  *f;
	char		szFile[MAX_QPATH];
	char		szLineBuffer[200];

	PurgeTextFilterInfo();

	// Create the pathname to the filter file.
	Com_sprintf(szFile, sizeof szFile, "%s/%s/%s/textfilter.cfg",
			basedir->string, game_dir->string, cfgdir->string);

	// Try to open the filter file.
	f = fopen( szFile, "rt");
	if ( !f)
	{
		if ( dedicated->value)
			gi.dprintf("Could not open text filter file.\n");
		
		// File not found.
		return;
	}

	// Iterate through the file and save the filter strings.
	while ( NULL != fgets_endws( szLineBuffer, sizeof(szLineBuffer), f) )
	{
		int nLength;
		char *psz;

		// Ignore any line which does not begin with "="
		if ( szLineBuffer[0] != '=')
			continue;

		// Allocate space to store the string.
		nLength = (int)strlen( szLineBuffer);

		if ( NULL == (psz = (char*) gi.TagMalloc( nLength, TAG_GAME)) )
		{
			safe_cprintf( NULL, PRINT_HIGH,
				"Could not allocate memory to store text filter string.\n");
			break;
		}

		// Is this a non-filter string?
		if ( szLineBuffer[1] == '!')
		{
			apszTextNonFilterStrings[ nTextNonFilterCount ] = psz;

			Strcpyn( apszTextNonFilterStrings[nTextNonFilterCount],
				szLineBuffer + 2, (int) nLength);

			if (++nTextNonFilterCount >= DIM(apszTextNonFilterStrings))
			{
				safe_cprintf( NULL, PRINT_HIGH,
					"Could not store more than %d text non-filter strings.\n", 
					DIM(apszTextNonFilterStrings) );
				break;
			}
		}
		else
		{
			apszTextFilterStrings[ nTextFilterCount ] = psz;

			Strcpyn( apszTextFilterStrings[nTextFilterCount], szLineBuffer + 1, (int) nLength);

			if ( ++nTextFilterCount >= DIM(apszTextFilterStrings) )
			{
				safe_cprintf( NULL, PRINT_HIGH,
					"Could not store more than %d text filter strings.\n", 
					DIM(apszTextFilterStrings) );
				break;
			}
		}
	}

	fclose( f);

	// Sort the strings by length so maximal matches will happen first.
	qsort( apszTextFilterStrings, nTextFilterCount,
		sizeof(apszTextFilterStrings[0]), CompareStringLength);

	qsort( apszTextNonFilterStrings, nTextNonFilterCount,
		sizeof(apszTextNonFilterStrings[0]), CompareStringLength);
}
