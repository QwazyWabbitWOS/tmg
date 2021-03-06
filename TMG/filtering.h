//
//  filtering.h
//

#ifndef FILTERING_H
#define FILTERING_H

/**
 Replaces bad strings in the text with an asterisk.
 */
qboolean FilterText(char *pszText);

/**
 Load textfilter.cfg file from current mod
 directory, fill the text filter string 
 arrays, then sort them.
 
 If no file exists, do nothing.
 */
void LoadTextFilterInfo(void);

#endif /* FILTERING_H */
