/* 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%                                                                             % 
%                                                                             % 
%                                                                             % 
%                  L       OOO    CCCC   AAA   L      EEEEE                   % 
%                  L      O   O  C      A   A  L      E                       % 
%                  L      O   O  C      AAAAA  L      EEE                     % 
%                  L      O   O  C      A   A  L      E                       % 
%                  LLLLL   OOO    CCCC  A   A  LLLLL  EEEEE                   % 
%                                                                             % 
%                                                                             % 
%                    ImageMagick Locale Message Methods                       % 
%                                                                             % 
%                                                                             % 
%                              Software Design                                % 
%                                John Cristy                                  % 
%                               September 2002                                % 
%                                                                             % 
%                                                                             % 
%  Copyright (C) 2002 ImageMagick Studio, a non-profit organization dedicated % 
%  to making software imaging solutions freely available.                     % 
%                                                                             % 
%  Permission is hereby granted, free of charge, to any person obtaining a    % 
%  copy of this software and associated documentation files ("ImageMagick"),  % 
%  to deal in ImageMagick without restriction, including without limitation   % 
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   % 
%  and/or sell copies of ImageMagick, and to permit persons to whom the       % 
%  ImageMagick is furnished to do so, subject to the following conditions:    % 
%                                                                             % 
%  The above copyright notice and this permission notice shall be included in % 
%  all copies or substantial portions of ImageMagick.                         % 
%                                                                             % 
%  Licensor ("ImageMagick Studio LLC") warrants that the copyright in and to  % 
%  the Software ("ImageMagick") is owned by ImageMagick Studio LLC or that    % 
%  ImageMagick is distributed by ImageMagick Studio LLC under a valid current % 
%  license. Except as expressly stated in the immediately preceding           % 
%  sentence, ImageMagick is provided by ImageMagick Studio LLC, contributors, % 
%  and copyright owners "AS IS", without warranty of any kind, express or     % 
%  implied, including but not limited to the warranties of merchantability,   % 
%  fitness for a particular purpose and non-infringement.  In no event shall  % 
%  ImageMagick Studio LLC, contributors or copyright owners be liable for     % 
%  any claim, damages, or other liability, whether in an action of contract,  % 
%  tort or otherwise, arising from, out of or in connection with              % 
%  ImageMagick.                                                               % 
%                                                                             % 
%  Except as contained in this notice, the name of the ImageMagick Studio     % 
%  shall not be used in advertising or otherwise to promote the sale, use or  % 
%  other dealings in ImageMagick without prior written authorization from the % 
%  ImageMagick Studio.                                                        % 
%                                                                             % 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
% 
% 
% 
*/ 
 
/* 
  Include declarations. 
*/ 
#include "studio.h" 
 
/* 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%                                                                             % 
%                                                                             % 
%                                                                             % 
%   G e t L o c a l e M e s s a g e                                           % 
%                                                                             % 
%                                                                             % 
%                                                                             % 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
% 
%  GetLocaleMessage() returns a message in the current locale that matches the 
%  supplied tag. 
% 
%  The format of the GetLocaleMessage method is: 
% 
%      const char *GetLocaleMessage(const char *tag) 
% 
%  A description of each parameter follows: 
% 
%    o tag: Return a message that matches this tag in the current locale. 
% 
% 
*/ 
/* This method is autogenerated-- do not edit */
const char *GetLocaleMessage(const char *tag)
{
   static const char *locale = 0;
   register const char *p, *tp, *np;
#  define NEXT_FIELD ((p = (np = strchr((tp = np), '/')) ? np++ : (np = tp + strlen(tp))), tp)

   if (!tag || *tag == '\0')
      return "";

   if ( (!locale &&
         ( (!(locale = setlocale(LC_CTYPE, 0)) || *locale == '\0') &&
           (!(locale = getenv("LC_ALL"))       || *locale == '\0') &&
           (!(locale = getenv("LC_CTYPE"))     || *locale == '\0') &&
           (!(locale = getenv("LANG"))         || *locale == '\0') ) )
         || !strcasecmp(locale, "C"))
      locale = "iso8859-1";

   tp = locale;
   p = locale + strlen(locale);
   np = tag;
  if (strncasecmp(locale, "iso8859-1", 9) || p - tp != 9)
    return tag;
  else
    switch (*NEXT_FIELD)
    {
    default:
      return tag;

    case 'c':  case 'C':
      if (p - tp == 5 && !strncasecmp(tp, "cache", 5))
        if (strncasecmp(NEXT_FIELD, "error", 5) || p - tp != 5)
          return tag;
        else
        if (strncasecmp(NEXT_FIELD, "pixel-cache-is-not-open", 23) || p - tp != 23)
          return tag;
        else
          return *np ? tag : "Pixel cache is not open";
      else
        return tag;

    case 'r':  case 'R':
      if (p - tp == 8 && !strncasecmp(tp, "resource", 8))
        if (strncasecmp(NEXT_FIELD, "limit", 5) || p - tp != 5)
          return tag;
        else
        if (strncasecmp(NEXT_FIELD, "error", 5) || p - tp != 5)
          return tag;
        else
          switch (*NEXT_FIELD)
          {
          default:
            return tag;

          case 'm':  case 'M':
            if (p - tp == 24 && !strncasecmp(tp, "memory-allocation-failed", 24))
              return *np ? tag : "Memory allocation failed";
            else
              return tag;

          case 'u':  case 'U':
            if (p - tp == 21 && !strncasecmp(tp, "unable-to-clone-image", 21))
              return *np ? tag : "Unable to clone image";
            else
              return tag;
          }
      else
        return tag;
    }

   return tag;
}
