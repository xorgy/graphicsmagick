/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%        DDDD   EEEEE  L      EEEEE   GGGG   AAA   TTTTT  EEEEE  SSSSS        %
%        D   D  E      L      E      G      A   A    T    E      SS           %
%        D   D  EEE    L      EEE    G  GG  AAAAA    T    EEE     SSS         %
%        D   D  E      L      E      G   G  A   A    T    E         SS        %
%        DDDD   EEEEE  LLLLL  EEEEE   GGG   A   A    T    EEEEE  SSSSS        %
%                                                                             %
%                                                                             %
%                   Methods to Read/Write/Invoke Delegates                    %
%                                                                             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               October 1998                                  %
%                                                                             %
%                                                                             %
%  Copyright (C) 2001 ImageMagick Studio, a non-profit organization dedicated %
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
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The Delegates methods associate a set of commands with a particular
%  image format.  ImageMagick uses delegates for formats it does not handle
%  directly.
%
%  Thanks to Bob Friesenhahn for the initial inspiration and design of the
%  delegates methods.
%
%
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"

/*
  Define declarations.
*/
#define DelegateFilename  "delegates.mgk"

/*
  Declare delegate map.
*/
static char
  *DelegateMap =
    "<?xml version=\"1.0\"?>"
    "<delegatemap>"
    "  <delgate />"
    "</delegatemap>";

/*
  Global declaractions.
*/
static DelegateInfo
  *delegate_list = (DelegateInfo *) NULL;

static SemaphoreInfo
  *delegate_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declaractions.
*/
static unsigned int
  ReadConfigurationFile(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y D e l e g a t e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DestroyDelegateInfo deallocates memory associated with the delegates
%  list.
%
%  The format of the DestroyDelegateInfo method is:
%
%      DestroyDelegateInfo(void)
%
*/
MagickExport void DestroyDelegateInfo(void)
{
  DelegateInfo
    *delegate;

  register DelegateInfo
    *p;

  AcquireSemaphoreInfo(&delegate_semaphore);
  for (p=delegate_list; p != (DelegateInfo *) NULL; )
  {
    if (p->filename != (char *) NULL)
      LiberateMemory((void **) &p->filename);
    if (p->decode != (char *) NULL)
      LiberateMemory((void **) &p->decode);
    if (p->encode != (char *) NULL)
      LiberateMemory((void **) &p->encode);
    if (p->commands != (char *) NULL)
      LiberateMemory((void **) &p->commands);
    delegate=p;
    p=p->next;
    LiberateMemory((void **) &delegate);
  }
  delegate_list=(DelegateInfo *) NULL;
  DestroySemaphoreInfo(delegate_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetDelegateInfo returns any delegates associated with the specified
%  tag.  
%
%  The format of the GetDelegateInfo method is:
%
%      DelegateInfo *GetDelegateInfo(const char *decode,const char *encode,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o delgate_info: Method GetDelegateInfo returns any delegates associated
%      with the specified tag.  
%
%    o decode: Specifies the decode delegate we are searching for as a
%      character string.
%
%    o encode: Specifies the encode delegate we are searching for as a
%      character string.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport DelegateInfo *GetDelegateInfo(const char *decode,
  const char *encode,ExceptionInfo *exception)
{
  register DelegateInfo
    *p;

  AcquireSemaphoreInfo(&delegate_semaphore);
  if (delegate_list == (DelegateInfo *) NULL)
    (void) ReadConfigurationFile(DelegateFilename,exception);
  LiberateSemaphoreInfo(&delegate_semaphore);
  if ((LocaleCompare(decode,"*") == 0) && (LocaleCompare(encode,"*") == 0))
    return(delegate_list);
  /*
    Search for requested delegate.
  */
  for (p=delegate_list; p != (DelegateInfo *) NULL; p=p->next)
  {
    if (p->mode > 0)
      {
        if (LocaleCompare(p->decode,decode) == 0)
          break;
        continue;
      }
    if (p->mode < 0)
      {
        if (LocaleCompare(p->encode,encode) == 0)
          break;
        continue;
      }
    if (LocaleCompare(decode,p->decode) == 0)
      if (LocaleCompare(encode,p->encode) == 0)
        break;
    if (LocaleCompare(decode,"*") == 0)
      if (LocaleCompare(encode,p->encode) == 0)
        break;
    if (LocaleCompare(decode,p->decode) == 0)
      if (LocaleCompare(encode,"*") == 0)
        break;
  }
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e C o m m a n d                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetDelegateCommand replaces any embedded formatting characters with
%  the appropriate image attribute and returns the resulting command.
%
%  The format of the GetDelegateCommand method is:
%
%      char *GetDelegateCommand(const ImageInfo *image_info,Image *image,
%        const char *decode,const char *encode)
%
%  A description of each parameter follows:
%
%    o command: Method GetDelegateCommand returns the command associated
%      with specified delegate tag.
%
%    o image_info: The imageInfo.
%
%    o image: The image.
%
%    o decode: Specifies the decode delegate we are searching for as a
%      character string.
%
%    o encode: Specifies the encode delegate we are searching for as a
%      character string.
%
%
*/
MagickExport char *GetDelegateCommand(const ImageInfo *image_info,Image *image,
  const char *decode,const char *encode)
{
  char
    *command,
    **commands;

  DelegateInfo
    *delegate_info;

  register int
    i;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(decode!= (char *) NULL);
  delegate_info=GetDelegateInfo(decode,encode,&image->exception);
  if (delegate_info == (DelegateInfo *) NULL)
    {
      ThrowException(&image->exception,MissingDelegateWarning,"no tag found",
        decode ? decode : encode);
      return((char *) NULL);
    }
  commands=StringToList(delegate_info->commands);
  if (commands == (char **) NULL)
    {
      ThrowException(&image->exception,ResourceLimitWarning,
        "Memory allocation failed",decode ? decode : encode);
      return((char *) NULL);
    }
  command=TranslateText(image_info,image,commands[0]);
  if (command == (char *) NULL)
    ThrowException(&image->exception,ResourceLimitWarning,
      "Memory allocation failed",commands[0]);
  /*
    Free resources.
  */
  for (i=0; commands[i] != (char *) NULL; i++)
    LiberateMemory((void **) &commands[i]);
  LiberateMemory((void **) &commands);
  return(command);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n v o k e D e l e g a t e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method InvokeDelegate replaces any embedded formatting characters with
%  the appropriate image attribute and executes the resulting command.  False
%  is returned if the commands execute with success otherwise True.
%
%  The format of the InvokeDelegate method is:
%
%      unsigned int InvokeDelegate(const ImageInfo *image_info,
%        Image *image,const char *decode,const char *encode)
%
%  A description of each parameter follows:
%
%    o image_info: The imageInfo.
%
%    o image: The image.
%
%
*/
MagickExport unsigned int InvokeDelegate(const ImageInfo *image_info,
  Image *image,const char *decode,const char *encode)
{
  char
    *command,
    **commands,
    filename[MaxTextExtent];

  DelegateInfo
    *delegate_info;

  register int
    i;

  unsigned int
    status;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) strcpy(filename,image->filename);
  delegate_info=GetDelegateInfo(decode,encode,&image->exception);
  if (delegate_info == (DelegateInfo *) NULL)
    ThrowBinaryException(MissingDelegateWarning,"no tag found",
      decode ? decode : encode);
  if (LocaleCompare(delegate_info->decode,"YUV") == 0)
    if ((LocaleCompare(delegate_info->encode,"M2V") == 0) ||
        (LocaleCompare(delegate_info->encode,"MPG") == 0) ||
        (LocaleCompare(delegate_info->encode,"MPEG") == 0))
      {
        FILE
          *file;

        unsigned int
          mpeg;

        /*
          Write parameter file (see mpeg2encode documentation for details).
        */
        CoalesceImages(image,&image->exception);
        mpeg=LocaleCompare(delegate_info->encode,"M2V") != 0;
        file=fopen(image_info->unique,"w");
        if (file == (FILE *) NULL)
          ThrowBinaryException(DelegateWarning,"delegate failed",
            decode ? decode : encode);
        (void) fprintf(file,"MPEG\n");
        (void) fprintf(file,"%.1024s%%d\n",image->filename);
        (void) fprintf(file,"-\n");
        (void) fprintf(file,"-\n");
        (void) fprintf(file,"-\n");
        (void) fprintf(file,"%.1024s\n",image_info->zero);
        (void) fprintf(file,"1\n");
        (void) fprintf(file,"%u\n",GetNumberScenes(image));
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"00:00:00:00\n");
        (void) fprintf(file,"%d\n",mpeg ? 12 : 15);
        (void) fprintf(file,"3\n");
        (void) fprintf(file,"%d\n",mpeg ? 1 : 0);
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"%u\n",image->columns+
          (image->columns & 0x01 ? 1 : 0));
        (void) fprintf(file,"%u\n",image->rows+(image->rows & 0x01 ? 1 : 0));
        (void) fprintf(file,"%d\n",mpeg ? 8 : 2);
        (void) fprintf(file,"%d\n",mpeg ? 3 : 5);
        (void) fprintf(file,"%.1f\n",mpeg ? 1152000.0 : 5000000.0);
        (void) fprintf(file,"%d\n",mpeg ? 20 : 112);
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"%d\n",mpeg ? 1 : 0);
        (void) fprintf(file,"4\n");
        (void) fprintf(file,"8\n");
        (void) fprintf(file,"%d\n",mpeg ? 1 : 0);
        (void) fprintf(file,"1\n");
        (void) fprintf(file,"%d\n",mpeg ? 1 : 2);
        (void) fprintf(file,"5\n");
        (void) fprintf(file,"5\n");
        (void) fprintf(file,"%d\n",mpeg ? 5 : 4);
        (void) fprintf(file,"%u\n",image->columns+
          (image->columns & 0x01 ? 1 : 0));
        (void) fprintf(file,"%u\n",image->rows+(image->rows & 0x01 ? 1 : 0));
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"%d\n",mpeg ? 0 : 1);
        (void) fprintf(file,"%d %d %d\n",mpeg ? 1 : 0,mpeg ? 1 : 0,
          mpeg ? 1 : 0);
        (void) fprintf(file,"0 0 0\n");
        (void) fprintf(file,"%d %d %d\n",mpeg ? 0 : 1,mpeg ? 0 : 1,
          mpeg ? 0 : 1);
        (void) fprintf(file,"%d 0 0\n",mpeg ? 0 : 1);
        (void) fprintf(file,"0 0 0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"%d\n",mpeg ? 1 : 0);
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"0\n");
        (void) fprintf(file,"2 2 11 11\n");
        (void) fprintf(file,"1 1 3 3\n");
        (void) fprintf(file,"1 1 7 7\n");
        (void) fprintf(file,"1 1 7 7\n");
        (void) fprintf(file,"1 1 3 3\n");
        (void) fclose(file);
        (void) strcat(image->filename,"%d.yuv");
      }
  if (delegate_info->mode != 0)
    if ((decode && (delegate_info->encode != (char *) NULL)) ||
        (encode && (delegate_info->decode != (char *) NULL)))
      {
        char
          filename[MaxTextExtent],
          *magick;

        ImageInfo
          *clone_info;

        register Image
          *p;

        /*
          Delegate requires a particular image format.
        */
        magick=TranslateText(image_info,image,decode != (char *) NULL ?
          delegate_info->encode : delegate_info->decode);
        if (magick == (char *) NULL)
          ThrowBinaryException(DelegateWarning,"delegate failed",
            decode ? decode : encode);
        LocaleUpper(magick);
        (void) strcpy((char *) image_info->magick,magick);
        (void) strcpy(image->magick,magick);
        LiberateMemory((void **) &magick);
        (void) strcpy(filename,image->filename);
        clone_info=CloneImageInfo(image_info);
        if (clone_info == (ImageInfo *) NULL)
          ThrowBinaryException(ResourceLimitWarning,"Memory allocation failed",
            (char *) NULL);
        FormatString(clone_info->filename,"%.1024s:",delegate_info->decode);
        SetImageInfo(clone_info,True,&image->exception);
        for (p=image; p != (Image *) NULL; p=p->next)
        {
          FormatString(p->filename,"%.1024s:%.1024s",delegate_info->decode,
            filename);
          status=WriteImage(clone_info,p);
          if (status == False)
            ThrowBinaryException(DelegateWarning,"delegate failed",
              decode ? decode : encode);
          if (clone_info->adjoin)
            break;
        }
        DestroyImageInfo(clone_info);
      }
  (void) strcpy(image->filename,filename);
  commands=StringToList(delegate_info->commands);
  if (commands == (char **) NULL)
    ThrowBinaryException(ResourceLimitWarning,"Memory allocation failed",
      decode ? decode : encode);
  command=(char *) NULL;
  status=True;
  for (i=0; commands[i] != (char *) NULL; i++)
  {
    status=True;
    command=TranslateText(image_info,image,commands[i]);
    if (command == (char *) NULL)
      break;
    /*
      Execute delegate.
    */
    if (delegate_info->spawn)
      ConcatenateString(&command," &");
    status=SystemCommand(image_info->verbose,command);
    LiberateMemory((void **) &command);
    if (status != False)
      ThrowBinaryException(DelegateWarning,"delegate failed",commands[i]);
    LiberateMemory((void **) &commands[i]);
  }
  /*
    Free resources.
  */
  (void) remove(image_info->unique);
  (void) remove(image_info->zero);
  for ( ; commands[i] != (char *) NULL; i++)
    LiberateMemory((void **) &commands[i]);
  LiberateMemory((void **) &commands);
  return(!status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t D e l e g a t e I n f o                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ListDelegateInfo lists the image formats to a file.
%
%  The format of the ListDelegateInfo method is:
%
%      unsigned int ListDelegateInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport unsigned int ListDelegateInfo(FILE *file,ExceptionInfo *exception)
{
  char
    delegate[MaxTextExtent],
    tag[MaxTextExtent];

  register DelegateInfo
    *p;

  register int
    i;

  if (file == (const FILE *) NULL)
    file=stdout;
  (void) fprintf(file,"ImageMagick defines these delegates to read orwrite "
    "image formats it does not\ndirectly support.\n");
  p=GetDelegateInfo("*","*",exception);
  if (p == (DelegateInfo *) NULL)
    return(False);
  if (p->filename != (char *) NULL)
    (void) fprintf(file,"\nFilename: %.1024s\n\n",p->filename);
  (void) fprintf(file,"Decode-Tag   Encode-Tag  Delegate\n");
  (void) fprintf(file,"-------------------------------------------------------"
    "------------------------\n");
  for ( ; p != (DelegateInfo *) NULL; p=p->next)
  {
    if (p->restrain)
      continue;
    i=0;
    if (p->commands != (char *) NULL)
      for (i=0; !isspace((int) p->commands[i]); i++)
        delegate[i]=p->commands[i];
    delegate[i]='\0';
    for (i=0; i < 10; i++)
      tag[i]=' ';
    tag[i]='\0';
    if (p->encode != (char *) NULL)
      (void) strncpy(tag,p->encode,Extent(p->encode));
    (void) fprintf(file,"%10s%.1024s=%.1024s%.1024s  %s\n",
      p->decode ? p->decode : "",p->mode <= 0 ? "<" : " ",
      p->mode >= 0 ? ">" : " ",tag,delegate);
  }
  (void) fflush(file);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d C o n f i g u r a t i o n F i l e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadConfigurationFile reads the color configuration file which maps
%  color strings with a particular image format.
%
%  The format of the ReadConfigurationFile method is:
%
%      unsigned int ReadConfigurationFile(const char *basename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o status: Method ReadConfigurationFile returns True if at least one color
%      is defined otherwise False.
%
%    o basename:  The color configuration filename.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
static unsigned int ReadConfigurationFile(const char *basename,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent],
    keyword[MaxTextExtent],
    *path,
    *q,
    *token,
    *xml;

  size_t
    length;

  /*
    Read the delegates configuration file.
  */
  FormatString(filename,"%.1024s",basename);
  path=GetMagickConfigurePath(basename);
  if (path != (char *) NULL)
    {
      FormatString(filename,"%.1024s",path);
      LiberateMemory((void **) &path);
    }
  xml=(char *) FileToBlob(filename,&length,exception);
  if (xml == (char *) NULL)
    xml=DelegateMap;
  token=AllocateString(xml);
  for (q=xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetToken(q,&q,token);
    if (*token == '\0')
      break;
    FormatString(keyword,"%.1024s",token);
    if (LocaleCompare(keyword,"<!") == 0)
      {
        /*
          Comment.
        */
        while ((*token != '>') && (*q != '\0'))
          GetToken(q,&q,token);
        continue;
      }
    if (LocaleCompare(keyword,"<delegate") == 0)
      {
        DelegateInfo
          *delegate_info;

        /*
          Allocate memory for the delegate list.
        */
        delegate_info=(DelegateInfo *) AcquireMemory(sizeof(DelegateInfo));
        if (delegate_info == (DelegateInfo *) NULL)
          MagickError(ResourceLimitError,"Unable to allocate delegates",
            "Memory allocation failed");
        memset(delegate_info,0,sizeof(DelegateInfo));
        if (delegate_list == (DelegateInfo *) NULL)
          {
            delegate_info->filename=AllocateString(filename);
            delegate_list=delegate_info;
            continue;
          }
        delegate_list->next=delegate_info;
        delegate_info->previous=delegate_list;
        delegate_list=delegate_list->next;
        continue;
      }
    if (delegate_list == (DelegateInfo *) NULL)
      continue;
    GetToken(q,(char **) NULL,token);
    if (*token != '=')
      continue;
    GetToken(q,&q,token);
    GetToken(q,&q,token);
    switch (*keyword) 
    {
      case 'C':
      case 'c':
      {
        if (LocaleCompare((char *) keyword,"command") == 0)
          {
            delegate_list->commands=AllocateString(token);
            break;
          }
        break;
      }
      case 'D':
      case 'd':
      {
        if (LocaleCompare((char *) keyword,"decode") == 0)
          {
            delegate_list->decode=AllocateString(token);
            delegate_list->mode=1;
            break;
          }
        break;
      }
      case 'E':
      case 'e':
      {
        if (LocaleCompare((char *) keyword,"encode") == 0)
          {
            delegate_list->encode=AllocateString(token);
            delegate_list->mode=(-1);
            break;
          }
        break;
      }
      case 'M':
      case 'm':
      {
        if (LocaleCompare((char *) keyword,"mode") == 0)
          {
            delegate_list->mode=1;
            if (LocaleCompare(token,"bi") == 0)
              delegate_list->mode=0;
            else
              if (LocaleCompare(token,"encode") == 0)
                delegate_list->mode=(-1);
            break;
          }
        break;
      }
      case 'R':
      case 'r':
      {
        if (LocaleCompare((char *) keyword,"restrain") == 0)
          {
            delegate_list->restrain=LocaleCompare(token,"True") == 0;
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"spawn") == 0)
          {
            delegate_list->spawn=LocaleCompare(token,"True") == 0;
            break;
          }
        break;
      }
      default:
        break;
    }
  }
  LiberateMemory((void **) &token);
  if (xml != DelegateMap)
    LiberateMemory((void **) &xml);
  if (delegate_list == (DelegateInfo *) NULL)
    return(False);
  while (delegate_list->previous != (DelegateInfo *) NULL)
    delegate_list=delegate_list->previous;
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t D e l e g a t e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SetDelegateInfo adds or replaces a delegate in the delegate list and
%  returns the address of the first delegate.  If the delegate is NULL, just
%  the address of the first delegate is returned.
%
%  The format of the SetDelegateInfo method is:
%
%      DelegateInfo *SetDelegateInfo(DelegateInfo *delegate_info)
%
%  A description of each parameter follows:
%
%    o delegate_info: Method SetDelegateInfo returns the address of the
%      first delegate in the delegates list.
%
%    o delegate_info:  A structure of type DelegateInfo.  This information
%      is added to the end of the delegates linked-list.
%
%
*/
MagickExport DelegateInfo *SetDelegateInfo(DelegateInfo *delegate_info)
{
  DelegateInfo
    *delegate;

  register DelegateInfo
    *p;

  /*
    Initialize new delegate.
  */
  assert(delegate_info != (DelegateInfo *) NULL);
  delegate=(DelegateInfo *) AcquireMemory(sizeof(DelegateInfo));
  if (delegate == (DelegateInfo *) NULL)
    return(delegate_list);
  (void) strcpy(delegate->decode,delegate_info->decode);
  (void) strcpy(delegate->encode,delegate_info->encode);
  delegate->mode=delegate_info->mode;
  delegate->commands=(char *) NULL;
  if (delegate_info->commands != (char *) NULL)
    {
      /*
        Note commands associated with this delegate.
      */
      delegate->commands=(char *)
        AcquireMemory(Extent(delegate_info->commands)+1);
      if (delegate->commands == (char *) NULL)
        return(delegate_list);
      (void) strcpy(delegate->commands,delegate_info->commands);
    }
  delegate->previous=(DelegateInfo *) NULL;
  delegate->next=(DelegateInfo *) NULL;
  if (delegate_list == (DelegateInfo *) NULL)
    {
      delegate_list=delegate;
      return(delegate_list);
    }
  for (p=delegate_list; p != (DelegateInfo *) NULL; p=p->next)
  {
    if ((LocaleCompare(p->decode,delegate_info->decode) == 0) &&
        (LocaleCompare(p->encode,delegate_info->encode) == 0) &&
        (p->mode == delegate_info->mode))
      {
        /*
          Delegate overrides an existing one with the same tags.
        */
        LiberateMemory((void **) &p->commands);
        p->commands=delegate->commands;
        LiberateMemory((void **) &delegate);
        return(delegate_list);
      }
    if (p->next == (DelegateInfo *) NULL)
      break;
  }
  /*
    Place new delegate at the end of the delegate list.
  */
  delegate->previous=p;
  p->next=delegate;
  return(delegate_list);
}
