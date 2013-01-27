#include "g_local.h"
#include <stdio.h>
#include <string.h>
#define TEXT_FILTER_FILE		"textfilter.cfg"
#define DIM(a) ( sizeof(a) / sizeof(a[0]) )

//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
/*
typedef struct
{
	//
	//	...
	//

	char				  *apszTextFilterStrings[1000];
	int					nTextFilterCount;

	char				  *apszTextNonFilterStrings[1000];
	int					nTextNonFilterCount;

} game_locals_t;

/**************************************************************************/

void Strcpyn(
	char			*pszDest,
	const char	*pszSrc,
	int			 nDestSize)
{
	strncpy( pszDest, pszSrc, nDestSize);
	pszDest[nDestSize-1] = '\0';
}

/**************************************************************************/

char *strstri(
	char *str1,
	char *str2)
{
	char *cp = (char *) str1;
	char *s1, *s2;

	if ( !*str2 )
		return((char *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char *) str2;

		while ( *s1 && *s2 && !( tolower(*s1) - tolower(*s2) ) )
			s1++, s2++;

		if (!*s2)
   		return(cp);

  		 cp++;
	}

	return(NULL);
}

/**************************************************************************/

// Search for pszSubstring inside pszString, but consider identical
// characters between letters in pszSubstring to be a match.

char *StrStrSpanNoCase(
	char *pszString,
	char *pszSubstring)
{
	char *cp = (char *) pszString;
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
			int nCount = strlen( pszSubstring) - 1;

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

// Does an fgets() but strips any trailing whitespace.

char *fgets_endws(
	char *pszBuffer,
	int	nSize,
	FILE *pStream)
{
	char *psz;
	char *pszReturn = fgets( pszBuffer, nSize, pStream);

	if ( pszReturn == NULL)
		return NULL;

	// Iterate through the buffer and strip EOL characters.
	for ( psz = pszBuffer + strlen( pszBuffer); psz != pszBuffer; psz--)
	{
		if ( *psz == '\r' || *psz == '\n' || *psz == ' ' ||
			  *psz == '\t' || *psz == '\0')
		{
			*psz = '\0';
		}
		else
			break;
	}

	return pszReturn;
}

/**************************************************************************/

// Replaces bad strings in the text with an asterisk.
qboolean FilterText(
	char *pszText)
{
	int 	 i;
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
	for ( i = 0; i < game.nTextNonFilterCount; )
	{
		int nMatchLen;
		
		p = strstri( szBuffer, game.apszTextNonFilterStrings[i] );

		if ( p == NULL)
		{
			i++;
			continue;
		}
		
		// Got a match, so set the high bit on the characters.
		for ( nMatchLen = strlen( game.apszTextNonFilterStrings[i] );
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
	for ( i = 0; i < game.nTextFilterCount; )
	{
		int nMatchLen;
		
		p = strstri( szBuffer, game.apszTextFilterStrings[i] );
		
		// Got a match?
		if ( p != NULL)
		{
			// Got a match, so replace all of the matched characters with a
			// chr(1)
			for ( nMatchLen = strlen( game.apszTextFilterStrings[i] );
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
		if ( strlen( game.apszTextFilterStrings[i] ) > 2 &&
			  NULL != (p = StrStrSpanNoCase( szBuffer, game.apszTextFilterStrings[i])) )
		{
			// Replace matched characters with a chr(1)
			for ( nMatchLen = strlen( game.apszTextFilterStrings[i] );
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

void PurgeTextFilterInfo()
{
	int i;

	for ( i = 0; i < game.nTextFilterCount; i++)
	{
		gi.TagFree( game.apszTextFilterStrings[i] );
		game.apszTextFilterStrings[i] = NULL;
	}

	game.nTextFilterCount = 0;

	for ( i = 0; i < game.nTextNonFilterCount; i++)
	{
		gi.TagFree( game.apszTextNonFilterStrings[i] );
		game.apszTextNonFilterStrings[i] = NULL;
	}

	game.nTextNonFilterCount = 0;
}

/**************************************************************************/

int CompareStringLength(
	void const *a,
	void const *b)
{
	int n1, n2;

	n1 = strlen( *(char **) a);
	n2 = strlen( *(char **) b);

	// Reverse comparison so strings will be sorted in decreasing order.
	return n2 - n1;
}

/**************************************************************************/

void LoadTextFilterInfo()
{
	FILE	  *f;
	char		szFile[256];
	char		szLineBuffer[200];

	PurgeTextFilterInfo();

	// Create the pathname to the filter file.
#if defined(linux)
	sprintf(szFile, "%s/%s/%s/textfilter.cfg", basedir->string, game_dir->string, cfgdir->string);
#else
	sprintf(szFile, "%s\\%s\\%s\\textfilter.cfg", basedir->string, game_dir->string, cfgdir->string);
#endif
    


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
		int	nLength;
		char *psz;

		// Ignore any line which does not begin with "="
		if ( szLineBuffer[0] != '=')
			continue;

		// Allocate space to store the string.
		nLength = strlen( szLineBuffer);

		if ( NULL == (psz = malloc( nLength + 1)) )
		{
			safe_cprintf( NULL, PRINT_HIGH,
				"Could not allocate memory to store text filter string.\n");
			break;
		}

		// Is this a non-filter string?
		if ( szLineBuffer[1] == '!')
		{
			game.apszTextNonFilterStrings[ game.nTextNonFilterCount ] = psz;

			Strcpyn( game.apszTextNonFilterStrings[game.nTextNonFilterCount],
				szLineBuffer + 2, nLength + 1);

			if ( ++game.nTextNonFilterCount >= DIM(game.apszTextNonFilterStrings) )
			{
				safe_cprintf( NULL, PRINT_HIGH,
					"Could not store more than %d text non-filter strings.\n", 
					DIM(game.apszTextNonFilterStrings) );
				break;
			}
		}
		else
		{
			game.apszTextFilterStrings[ game.nTextFilterCount ] = psz;

			Strcpyn( game.apszTextFilterStrings[game.nTextFilterCount],
				szLineBuffer + 1, nLength + 1);

			if ( ++game.nTextFilterCount >= DIM(game.apszTextFilterStrings) )
			{
				safe_cprintf( NULL, PRINT_HIGH,
					"Could not store more than %d text filter strings.\n", 
					DIM(game.apszTextFilterStrings) );
				break;
			}
		}
	}

	fclose( f);

	// Sort the strings by length so maximal matches will happen first.
	qsort( game.apszTextFilterStrings, game.nTextFilterCount,
		sizeof(game.apszTextFilterStrings[0]), CompareStringLength);

	qsort( game.apszTextNonFilterStrings, game.nTextNonFilterCount,
		sizeof(game.apszTextNonFilterStrings[0]), CompareStringLength);
}


