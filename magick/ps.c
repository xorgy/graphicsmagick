/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                               PPPP   SSSSS                                  %
%                               P   P  SS                                     %
%                               PPPP    SSS                                   %
%                               P         SS                                  %
%                               P      SSSSS                                  %
%                                                                             %
%                                                                             %
%                    Read/Write ImageMagick Image Format.                     %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999 E. I. du Pont de Nemours and Company                        %
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
%  E. I. du Pont de Nemours and Company be liable for any claim, damages or   %
%  other liability, whether in an action of contract, tort or otherwise,      %
%  arising from, out of or in connection with ImageMagick or the use or other %
%  dealings in ImageMagick.                                                   %
%                                                                             %
%  Except as contained in this notice, the name of the E. I. du Pont de       %
%  Nemours and Company shall not be used in advertising or otherwise to       %
%  promote the sale, use or other dealings in ImageMagick without prior       %
%  written authorization from the E. I. du Pont de Nemours and Company.       %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
^L
/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"
#include "proxy.h"

/*
  Constant declarations.
*/
const char
  *AppendBinaryType = "ab",
  *PSDensityGeometry = "72x72",
  *PSPageGeometry = "612x792+43+43>";

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P S I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPSImage reads a Adobe Postscript image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPSImage routine is:
%
%      image=ReadPSImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPSImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadPSImage(const ImageInfo *image_info)
{
#define BoundingBox  "%%BoundingBox:"
#define DocumentMedia  "%%DocumentMedia:"
#define PageBoundingBox  "%%PageBoundingBox:"
#define PostscriptLevel  "%!PS-"
#define ShowPage  "showpage"

  char
    density[MaxTextExtent],
    command[MaxTextExtent],
    filename[MaxTextExtent],
    geometry[MaxTextExtent],
    options[MaxTextExtent],
    postscript_filename[MaxTextExtent],
    translate_geometry[MaxTextExtent];

  DelegateInfo
    delegate_info;

  double
    dx_resolution,
    dy_resolution;

  FILE
    *file;

  Image
    *image,
    *next_image;

  int
    c,
    count,
    status;

  long int
    filesize;

  RectangleInfo
    box,
    page_info;

  register char
    *p;

  register int
    i;

  SegmentInfo
    bounding_box;

  unsigned int
    eps_level,
    height,
    level,
    width;

  if (image_info->monochrome)
    {
      if (!GetDelegateInfo("gs-mono",(char *) NULL,&delegate_info))
        return((Image *) NULL);
    }
  else
    if (!GetDelegateInfo("gs-color",(char *) NULL,&delegate_info))
      return((Image *) NULL);
  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image_info,image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    ReaderExit(FileOpenWarning,"Unable to open file",image);
  /*
    Open temporary output file.
  */
  TemporaryFilename(postscript_filename);
  file=fopen(postscript_filename,WriteBinaryType);
  if (file == (FILE *) NULL)
    ReaderExit(FileOpenWarning,"Unable to write file",image);
  FormatString(translate_geometry,"%f %f translate\n              ",0.0,0.0);
  (void) fputs(translate_geometry,file);
  /*
    Set the page geometry.
  */
  dx_resolution=72.0;
  dy_resolution=72.0;
  if ((image->x_resolution == 0.0) || (image->y_resolution == 0.0))
    {
      (void) strcpy(density,PSDensityGeometry);
      count=sscanf(density,"%fx%f",&image->x_resolution,&image->y_resolution);
      if (count != 2)
        image->y_resolution=image->x_resolution;
    }
  FormatString(density,"%gx%g",image->x_resolution,image->y_resolution);
  page_info.width=612;
  page_info.height=792;
  page_info.x=0;
  page_info.y=0;
  (void) ParseImageGeometry(PSPageGeometry,&page_info.x,&page_info.y,
    &page_info.width,&page_info.height);
  /*
    Determine page geometry from the Postscript bounding box.
  */
  filesize=0;
  if (Latin1Compare(image_info->magick,"EPT") == 0)
    {
      /*
        Dos binary file header.
      */
      (void) LSBFirstReadLong(image->file);
      count=LSBFirstReadLong(image->file);
      filesize=LSBFirstReadLong(image->file);
      for (i=0; i < (count-12); i++)
        (void) fgetc(image->file);
    }
  box.width=0;
  box.height=0;
  /*
    Copy Postscript to temporary file.
  */
  level=0;
  eps_level=0;
  p=command;
  for (i=0; (Latin1Compare(image_info->magick,"EPT") != 0) || i < filesize; i++)
  {
    c=fgetc(image->file);
    if (c == EOF)
      break;
    (void) fputc(c,file);
    *p++=c;
    if ((c != '\n') && (c != '\r') && ((p-command) < (MaxTextExtent-1)))
      continue;
    *p='\0';
    p=command;
    if (strncmp(PostscriptLevel,command,Extent(PostscriptLevel)) == 0)
      (void) sscanf(command,"%%!PS-Adobe-%d.0 EPSF-%d.0",&level,&eps_level);
    if (strncmp(ShowPage,command,Extent(ShowPage)) == 0)
      eps_level=0;
    /*
      Parse a bounding box statement.
    */
    count=0;
    if (strncmp(BoundingBox,command,Extent(BoundingBox)) == 0)
      count=sscanf(command,"%%%%BoundingBox: %f %f %f %f",&bounding_box.x1,
        &bounding_box.y1,&bounding_box.x2,&bounding_box.y2);
    if (strncmp(DocumentMedia,command,Extent(DocumentMedia)) == 0)
      count=sscanf(command,"%%%%DocumentMedia: %*s %f %f",&bounding_box.x2,
        &bounding_box.y2)+2;
    if (strncmp(PageBoundingBox,command,Extent(PageBoundingBox)) == 0)
      count=sscanf(command,"%%%%PageBoundingBox: %f %f %f %f",
        &bounding_box.x1,&bounding_box.y1,&bounding_box.x2,&bounding_box.y2);
    if (count != 4)
      continue;
    if ((bounding_box.x1 > bounding_box.x2) ||
        (bounding_box.y1 > bounding_box.y2))
      continue;
    /*
      Set Postscript render geometry.
    */
    FormatString(translate_geometry,"%f %f translate\n",-bounding_box.x1,
      -bounding_box.y1);
    width=(unsigned int) (bounding_box.x2-bounding_box.x1+1);
    if ((float) ((int) bounding_box.x2) != bounding_box.x2)
      width++;
    height=(unsigned int) (bounding_box.y2-bounding_box.y1+1);
    if ((float) ((int) bounding_box.y2) != bounding_box.y2)
      height++;
    if ((width <= box.width) && (height <= box.height))
      continue;
    page_info.width=width;
    page_info.height=height;
    box=page_info;
  }
  if (eps_level != 0)
    (void) fputs("showpage\n",file);
  if (image_info->page != (char *) NULL)
    (void) ParseImageGeometry(image_info->page,&page_info.x,&page_info.y,
      &page_info.width,&page_info.height);
  FormatString(geometry,"%ux%u",(unsigned int) (((page_info.width*
    image->x_resolution)/dx_resolution)+0.5),(unsigned int)
    (((page_info.height*image->y_resolution)/dy_resolution)+0.5));
  if (ferror(file))
    {
      MagickWarning(FileOpenWarning,"An error has occurred writing to file",
        postscript_filename);
      (void) fclose(file);
      return((Image *) NULL);
    }
  (void) fseek(file,0L,SEEK_SET);
  (void) fputs(translate_geometry,file);
  (void) fclose(file);
  CloseImage(image);
  filesize=image->filesize;
  DestroyImage(image);
  /*
    Use Ghostscript to convert Postscript image.
  */
  *options='\0';
  if (image_info->subrange != 0)
    FormatString(options,"-dFirstPage=%u -dLastPage=%u",
      image_info->subimage+1,image_info->subimage+image_info->subrange);
  (void) strcpy(filename,image_info->filename);
  TemporaryFilename((char *) image_info->filename);
  FormatString(command,delegate_info.commands,image_info->antialias ? 1 : 1,
    image_info->antialias ? 4 : 1,geometry,density,options,image_info->filename,
    postscript_filename);
  ProgressMonitor(RenderPostscriptText,0,8);
  status=SystemCommand(image_info->verbose,command);
  if (!IsAccessible(image_info->filename))
    {
      /*
        Ghostscript requires a showpage operator.
      */
      file=fopen(postscript_filename,AppendBinaryType);
      if (file == (FILE *) NULL)
        ReaderExit(FileOpenWarning,"Unable to write file",image);
      (void) fputs("showpage\n",file);
      (void) fclose(file);
      status=SystemCommand(image_info->verbose,command);
    }
  (void) remove(postscript_filename);
  ProgressMonitor(RenderPostscriptText,7,8);
  if (status)
    {
      /*
        Ghostscript has failed-- try the Display Postscript Extension.
      */
      (void) strcpy((char *) image_info->filename,filename);
      image=ReadDPSImage(image_info);
      if (image != (Image *) NULL)
        return(image);
      MagickWarning(CorruptImageWarning,"Postscript delegation failed",
        image_info->filename);
      return((Image *) NULL);
    }
  image=ReadPNMImage(image_info);
  (void) remove(image_info->filename);
  if (image == (Image *) NULL)
    {
      MagickWarning(CorruptImageWarning,"Postscript delegation failed",
        image_info->filename);
      return((Image *) NULL);
    }
  (void) strcpy((char *) image_info->filename,filename);
  do
  {
    (void) strcpy(image->magick,"PS");
    (void) strcpy(image->filename,image_info->filename);
    image->filesize=filesize;
    next_image=image->next;
    if (next_image != (Image *) NULL)
      image=next_image;
  } while (next_image != (Image *) NULL);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WritePSImage translates an image to encapsulated Postscript
%  Level I for printing.  If the supplied geometry is null, the image is
%  centered on the Postscript page.  Otherwise, the image is positioned as
%  specified by the geometry.
%
%  The format of the WritePSImage routine is:
%
%      status=WritePSImage(image_info,image)
%
%  A description of each parameter follows:
%
%    o status: Method WritePSImage return True if the image is printed.
%      False is returned if the image file cannot be opened for printing.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export unsigned int WritePSImage(const ImageInfo *image_info,Image *image)
{
  static const char
    *PostscriptProlog[]=
    {
      "%%BeginProlog",
      "%",
      "% Display a color image.  The image is displayed in color on",
      "% Postscript viewers or printers that support color, otherwise",
      "% it is displayed as grayscale.",
      "%",
      "/buffer 512 string def",
      "/byte 1 string def",
      "/color_packet 3 string def",
      "/pixels 768 string def",
      "",
      "/DirectClassPacket",
      "{",
      "  %",
      "  % Get a DirectClass packet.",
      "  %",
      "  % Parameters:",
      "  %   red.",
      "  %   green.",
      "  %   blue.",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile color_packet readhexstring pop pop",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 3 def",
      "  }",
      "  {",
      "    currentfile byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add 3 mul def",
      "  } ifelse",
      "  0 3 number_pixels 1 sub",
      "  {",
      "    pixels exch color_packet putinterval",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/DirectClassImage",
      "{",
      "  %",
      "  % Display a DirectClass image.",
      "  %",
      "  systemdict /colorimage known",
      "  {",
      "    columns rows 8",
      "    [",
      "      columns 0 0",
      "      rows neg 0 rows",
      "    ]",
      "    { DirectClassPacket } false 3 colorimage",
      "  }",
      "  {",
      "    %",
      "    % No colorimage operator;  convert to grayscale.",
      "    %",
      "    columns rows 8",
      "    [",
      "      columns 0 0",
      "      rows neg 0 rows",
      "    ]",
      "    { GrayDirectClassPacket } image",
      "  } ifelse",
      "} bind def",
      "",
      "/GrayDirectClassPacket",
      "{",
      "  %",
      "  % Get a DirectClass packet;  convert to grayscale.",
      "  %",
      "  % Parameters:",
      "  %   red",
      "  %   green",
      "  %   blue",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile color_packet readhexstring pop pop",
      "  color_packet 0 get 0.299 mul",
      "  color_packet 1 get 0.587 mul add",
      "  color_packet 2 get 0.114 mul add",
      "  cvi",
      "  /gray_packet exch def",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 1 def",
      "  }",
      "  {",
      "    currentfile byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add def",
      "  } ifelse",
      "  0 1 number_pixels 1 sub",
      "  {",
      "    pixels exch gray_packet put",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/GrayPseudoClassPacket",
      "{",
      "  %",
      "  % Get a PseudoClass packet;  convert to grayscale.",
      "  %",
      "  % Parameters:",
      "  %   index: index into the colormap.",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile byte readhexstring pop 0 get",
      "  /offset exch 3 mul def",
      "  /color_packet colormap offset 3 getinterval def",
      "  color_packet 0 get 0.299 mul",
      "  color_packet 1 get 0.587 mul add",
      "  color_packet 2 get 0.114 mul add",
      "  cvi",
      "  /gray_packet exch def",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 1 def",
      "  }",
      "  {",
      "    currentfile byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add def",
      "  } ifelse",
      "  0 1 number_pixels 1 sub",
      "  {",
      "    pixels exch gray_packet put",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/PseudoClassPacket",
      "{",
      "  %",
      "  % Get a PseudoClass packet.",
      "  %",
      "  % Parameters:",
      "  %   index: index into the colormap.",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile byte readhexstring pop 0 get",
      "  /offset exch 3 mul def",
      "  /color_packet colormap offset 3 getinterval def",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 3 def",
      "  }",
      "  {",
      "    currentfile byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add 3 mul def",
      "  } ifelse",
      "  0 3 number_pixels 1 sub",
      "  {",
      "    pixels exch color_packet putinterval",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/PseudoClassImage",
      "{",
      "  %",
      "  % Display a PseudoClass image.",
      "  %",
      "  % Parameters:",
      "  %   class: 0-PseudoClass or 1-Grayscale.",
      "  %",
      "  currentfile buffer readline pop",
      "  token pop /class exch def pop",
      "  class 0 gt",
      "  {",
      "    currentfile buffer readline pop",
      "    token pop /depth exch def pop",
      "    /grays columns 8 add depth sub depth mul 8 idiv string def",
      "    columns rows depth",
      "    [",
      "      columns 0 0",
      "      rows neg 0 rows",
      "    ]",
      "    { currentfile grays readhexstring pop } image",
      "  }",
      "  {",
      "    %",
      "    % Parameters:",
      "    %   colors: number of colors in the colormap.",
      "    %   colormap: red, green, blue color packets.",
      "    %",
      "    currentfile buffer readline pop",
      "    token pop /colors exch def pop",
      "    /colors colors 3 mul def",
      "    /colormap colors string def",
      "    currentfile colormap readhexstring pop pop",
      "    systemdict /colorimage known",
      "    {",
      "      columns rows 8",
      "      [",
      "        columns 0 0",
      "        rows neg 0 rows",
      "      ]",
      "      { PseudoClassPacket } false 3 colorimage",
      "    }",
      "    {",
      "      %",
      "      % No colorimage operator;  convert to grayscale.",
      "      %",
      "      columns rows 8",
      "      [",
      "        columns 0 0",
      "        rows neg 0 rows",
      "      ]",
      "      { GrayPseudoClassPacket } image",
      "    } ifelse",
      "  } ifelse",
      "} bind def",
      "",
      "/DisplayImage",
      "{",
      "  %",
      "  % Display a DirectClass or PseudoClass image.",
      "  %",
      "  % Parameters:",
      "  %   x & y translation.",
      "  %   x & y scale.",
      "  %   label pointsize.",
      "  %   image label.",
      "  %   image columns & rows.",
      "  %   class: 0-DirectClass or 1-PseudoClass.",
      "  %   compression: 0-RunlengthEncodedCompression or 1-NoCompression.",
      "  %   hex color packets.",
      "  %",
      "  gsave",
      "  currentfile buffer readline pop",
      "  token pop /x exch def",
      "  token pop /y exch def pop",
      "  x y translate",
      "  currentfile buffer readline pop",
      "  token pop /x exch def",
      "  token pop /y exch def pop",
      "  currentfile buffer readline pop",
      "  token pop /pointsize exch def pop",
      "  /Helvetica findfont pointsize scalefont setfont",
      (char *) NULL
    },
    *PostscriptEpilog[]=
    {
      "  x y scale",
      "  currentfile buffer readline pop",
      "  token pop /columns exch def",
      "  token pop /rows exch def pop",
      "  currentfile buffer readline pop",
      "  token pop /class exch def pop",
      "  currentfile buffer readline pop",
      "  token pop /compression exch def pop",
      "  class 0 gt { PseudoClassImage } { DirectClassImage } ifelse",
      "  grestore",
      (char *) NULL
    };

  char
    date[MaxTextExtent],
    density[MaxTextExtent],
    **labels;

  const char
    **q;

  double
    x_scale,
    y_scale;

  float
    dx_resolution,
    dy_resolution,
    x_resolution,
    y_resolution;

  int
    length,
    x,
    y;

  register RunlengthPacket
    *p;

  register int
    i,
    j;

  SegmentInfo
    bounding_box;

  time_t
    timer;

  unsigned int
    bit,
    byte,
    count,
    height,
    page,
    polarity,
    scene,
    text_size,
    width;

  /*
    Open output image file.
  */
  OpenImage(image_info,image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  page=1;
  scene=0;
  do
  {
    /*
      Scale image to size of Postscript page.
    */
    TransformRGBImage(image,RGBColorspace);
    text_size=0;
    if (image->label != (char *) NULL)
      text_size=MultilineCensus(image->label)*image_info->pointsize+12;
    width=image->columns;
    height=image->rows;
    x=0;
    y=text_size;
    if (image_info->page != (char *) NULL)
      (void) ParseImageGeometry(image_info->page,&x,&y,&width,&height);
    else
      if (image->page != (char *) NULL)
        (void) ParseImageGeometry(image->page,&x,&y,&width,&height);
      else
        if (Latin1Compare(image_info->magick,"PS") == 0)
          (void) ParseImageGeometry(PSPageGeometry,&x,&y,&width,&height);
    /*
      Scale relative to dots-per-inch.
    */
    dx_resolution=72.0;
    dy_resolution=72.0;
    x_resolution=72.0;
    (void) strcpy(density,PSDensityGeometry);
    count=sscanf(density,"%fx%f",&x_resolution,&y_resolution);
    if (count != 2)
      y_resolution=x_resolution;
    if (image_info->density != (char *) NULL)
      {
        count=sscanf(image_info->density,"%fx%f",&x_resolution,&y_resolution);
        if (count != 2)
          y_resolution=x_resolution;
      }
    x_scale=(width*dx_resolution)/x_resolution;
    width=(unsigned int) (x_scale+0.5);
    y_scale=(height*dy_resolution)/y_resolution;
    height=(unsigned int) (y_scale+0.5);
    if (page == 1)
      {
        /*
          Output Postscript header.
        */
        if (Latin1Compare(image_info->magick,"PS") == 0)
          (void) fprintf(image->file,"%%!PS-Adobe-3.0\n");
        else
          (void) fprintf(image->file,"%%!PS-Adobe-3.0 EPSF-3.0\n");
        (void) fprintf(image->file,"%%%%Creator: (ImageMagick)\n");
        (void) fprintf(image->file,"%%%%Title: (%.1024s)\n",image->filename);
        timer=time((time_t *) NULL);
        (void) localtime(&timer);
        (void) strcpy(date,ctime(&timer));
        date[Extent(date)-1]='\0';
        (void) fprintf(image->file,"%%%%CreationDate: (%.1024s)\n",date);
        bounding_box.x1=x;
        bounding_box.y1=y;
        bounding_box.x2=x+x_scale-1;
        bounding_box.y2=y+(height+text_size)-1;
        if (image_info->adjoin && (image->next != (Image *) NULL))
          (void) fprintf(image->file,"%%%%BoundingBox: (atend)\n");
        else
          (void) fprintf(image->file,"%%%%BoundingBox: %g %g %g %g\n",
            bounding_box.x1,bounding_box.y1,bounding_box.x2,bounding_box.y2);
        if (image->label != (char *) NULL)
          (void) fprintf(image->file,
            "%%%%DocumentNeededResources: font Helvetica\n");
        (void) fprintf(image->file,"%%%%DocumentData: Clean7Bit\n");
        (void) fprintf(image->file,"%%%%LanguageLevel: 1\n");
        if (Latin1Compare(image_info->magick,"PS") != 0)
          (void) fprintf(image->file,"%%%%Pages: 0\n");
        else
          {
            Image
              *next_image;

            unsigned int
              pages;

            /*
              Compute the number of pages.
            */
            (void) fprintf(image->file,"%%%%Orientation: Portrait\n");
            (void) fprintf(image->file,"%%%%PageOrder: Ascend\n");
            pages=1;
            if (image_info->adjoin)
              for (next_image=image->next; next_image != (Image *) NULL; )
              {
                next_image=next_image->next;
                pages++;
              }
            (void) fprintf(image->file,"%%%%Pages: %u\n",pages);
          }
        (void) fprintf(image->file,"%%%%EndComments\n");
        (void) fprintf(image->file,"\n%%%%BeginDefaults\n");
        (void) fprintf(image->file,"%%%%PageOrientation: Portrait\n");
        (void) fprintf(image->file,"%%%%EndDefaults\n\n");
        if ((Latin1Compare(image_info->magick,"EPI") == 0) ||
            (Latin1Compare(image_info->magick,"EPT") == 0) ||
            (Latin1Compare(image_info->magick,"EPSI") == 0))
          {
            Image
              *preview_image;

            /*
              Create preview image.
            */
            image->orphan=True;
            preview_image=CloneImage(image,image->columns,image->rows,True);
            image->orphan=False;
            if (preview_image == (Image *) NULL)
              WriterExit(ResourceLimitWarning,"Memory allocation failed",
                image);
            /*
              Dump image as bitmap.
            */
            if (!IsMonochromeImage(preview_image))
              {
                QuantizeInfo
                  quantize_info;

                GetQuantizeInfo(&quantize_info);
                quantize_info.number_colors=2;
                quantize_info.dither=image_info->dither;
                quantize_info.colorspace=GRAYColorspace;
                (void) QuantizeImage(&quantize_info,preview_image);
                SyncImage(preview_image);
              }
            polarity=Intensity(preview_image->colormap[0]) < (MaxRGB >> 1);
            if (preview_image->colors == 2)
              polarity=Intensity(preview_image->colormap[0]) >
                Intensity(preview_image->colormap[1]);
            bit=0;
            byte=0;
            count=0;
            x=0;
            p=preview_image->pixels;
            (void) fprintf(image->file,"%%%%BeginPreview: %u %u %u %u\n%%  ",
              preview_image->columns,preview_image->rows,(unsigned int) 1,
              (((preview_image->columns+7) >> 3)*preview_image->rows+35)/36);
            for (i=0; i < (int) preview_image->packets; i++)
            {
              for (j=0; j <= ((int) p->length); j++)
              {
                byte<<=1;
                if (p->index == polarity)
                  byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    (void) fprintf(image->file,"%02x",byte & 0xff);
                    count++;
                    if (count == 36)
                      {
                        (void) fprintf(image->file,"\n%%  ");
                        count=0;
                      };
                    bit=0;
                    byte=0;
                  }
                x++;
                if (x == (int) preview_image->columns)
                  {
                    if (bit != 0)
                      {
                        byte<<=(8-bit);
                        (void) fprintf(image->file,"%02x",byte & 0xff);
                        count++;
                        if (count == 36)
                          {
                            (void) fprintf(image->file,"\n%%  ");
                            count=0;
                          };
                        bit=0;
                        byte=0;
                      };
                    x=0;
                  }
                }
                p++;
              }
              (void) fprintf(image->file,"\n%%%%EndPreview\n");
              DestroyImage(preview_image);
            }
        /*
          Output Postscript commands.
        */
        for (q=PostscriptProlog; *q; q++)
          (void) fprintf(image->file,"%.255s\n",*q);
        for (i=MultilineCensus(image->label)-1; i >= 0; i--)
        {
          (void) fprintf(image->file,"  /label 512 string def\n");
          (void) fprintf(image->file,"  currentfile label readline pop\n");
          (void) fprintf(image->file,"  0 y %d add moveto label show pop\n",
            i*image_info->pointsize+12);
        }
        for (q=PostscriptEpilog; *q; q++)
          (void) fprintf(image->file,"%.255s\n",*q);
        if (Latin1Compare(image_info->magick,"PS") == 0)
          (void) fprintf(image->file,"  showpage\n");
        (void) fprintf(image->file,"} bind def\n");
        (void) fprintf(image->file,"%%%%EndProlog\n");
      }
    (void) fprintf(image->file,"%%%%Page:  1 %u\n",page++);
    (void) fprintf(image->file,"%%%%PageBoundingBox: %d %d %d %d\n",x,y,
      x+(int) width,y+(int) (height+text_size));
    if (x < bounding_box.x1)
      bounding_box.x1=x;
    if (y < bounding_box.y1)
      bounding_box.y1=y;
    if ((x+(int) width-1) > bounding_box.x2)
      bounding_box.x2=x+width-1;
    if ((y+(int) (height+text_size)-1) > bounding_box.y2)
      bounding_box.y2=y+(height+text_size)-1;
    if (image->label != (char *) NULL)
      (void) fprintf(image->file,"%%%%PageResources: font Helvetica\n");
    if (Latin1Compare(image_info->magick,"PS") != 0)
      (void) fprintf(image->file,"userdict begin\n");
    (void) fprintf(image->file,"%%%%BeginData:\n");
    (void) fprintf(image->file,"DisplayImage\n");
    /*
      Output image data.
    */
    labels=StringToList(image->label);
    (void) fprintf(image->file,"%d %d\n%f %f\n%u\n",x,y,x_scale,y_scale,
      image_info->pointsize);
    if (labels != (char **) NULL)
      {
        for (i=0; labels[i] != (char *) NULL; i++)
        {
          (void) fprintf(image->file,"%.1024s \n",labels[i]);
          FreeMemory(labels[i]);
        }
        FreeMemory(labels);
      }
    x=0;
    p=image->pixels;
    if (!IsPseudoClass(image) && !IsGrayImage(image))
      {
        /*
          Dump DirectClass image.
        */
        (void) fprintf(image->file,"%u %u\n%d\n%d\n",
          image->columns,image->rows,(int) (image->class == PseudoClass),
          (int) (image_info->compression == NoCompression));
        switch (image_info->compression)
        {
          case RunlengthEncodedCompression:
          default:
          {
            /*
              Dump runlength-encoded DirectColor packets.
            */
            for (i=0; i < (int) image->packets; i++)
            {
              for (length=p->length; length >= 0; length-=256)
              {
                if (image->matte && (p->index == Transparent))
                  (void) fprintf(image->file,"ffffff%02x",(unsigned int)
                    Min(length,0xff));
                else
                  (void) fprintf(image->file,"%02lx%02lx%02lx%02lx",
                    DownScale(p->red),DownScale(p->green),DownScale(p->blue),
                    (unsigned long) Min(length,0xff));
                x++;
                if (x == 9)
                  {
                    x=0;
                    (void) fprintf(image->file,"\n");
                  }
              }
              p++;
              if (image->previous == (Image *) NULL)
                if (QuantumTick(i,image->packets))
                  ProgressMonitor(SaveImageText,i,image->packets);
            }
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed DirectColor packets.
            */
            for (i=0; i < (int) image->packets; i++)
            {
              for (j=0; j <= ((int) p->length); j++)
              {
                if (image->matte && (p->index == Transparent))
                  (void) fprintf(image->file,"ffffff");
                else
                  (void) fprintf(image->file,"%02lx%02lx%02lx",
                    DownScale(p->red),DownScale(p->green),DownScale(p->blue));
                x++;
                if (x == 12)
                  {
                    x=0;
                    (void) fprintf(image->file,"\n");
                  }
              }
              p++;
              if (image->previous == (Image *) NULL)
                if (QuantumTick(i,image->packets))
                  ProgressMonitor(SaveImageText,i,image->packets);
            }
            break;
          }
        }
        (void) fprintf(image->file,"\n");
      }
    else
      if (IsGrayImage(image))
        {
          (void) fprintf(image->file,"%u %u\n1\n1\n1\n%d\n",
            image->columns,image->rows,IsMonochromeImage(image) ? 1 : 8);
          if (!IsMonochromeImage(image))
            {
              /*
                Dump image as grayscale.
              */
              for (i=0; i < (int) image->packets; i++)
              {
                for (j=0; j <= ((int) p->length); j++)
                {
                  (void) fprintf(image->file,"%02lx",DownScale(p->red));
                  x++;
                  if (x == 36)
                    {
                      x=0;
                      (void) fprintf(image->file,"\n");
                    }
                }
                p++;
                if (image->previous == (Image *) NULL)
                  if (QuantumTick(i,image->packets))
                    ProgressMonitor(SaveImageText,i,image->packets);
              }
            }
          else
            {
              int
                y;

              /*
                Dump image as bitmap.
              */
              polarity=Intensity(image->colormap[0]) > (MaxRGB >> 1);
              if (image->colors == 2)
                polarity=
                  Intensity(image->colormap[1]) > Intensity(image->colormap[0]);
              bit=0;
              byte=0;
              count=0;
              y=0;
              for (i=0; i < (int) image->packets; i++)
              {
                for (j=0; j <= ((int) p->length); j++)
                {
                  byte<<=1;
                  if (p->index == polarity)
                    byte|=0x01;
                  bit++;
                  if (bit == 8)
                    {
                      (void) fprintf(image->file,"%02x",byte & 0xff);
                      count++;
                      if (count == 36)
                        {
                          (void) fprintf(image->file,"\n");
                          count=0;
                        };
                      bit=0;
                      byte=0;
                    }
                  x++;
                  if (x == (int) image->columns)
                    {
                      /*
                        Advance to the next scanline.
                      */
                      if (bit != 0)
                        {
                          byte<<=(8-bit);
                          (void) fprintf(image->file,"%02x",byte & 0xff);
                          count++;
                          if (count == 36)
                            {
                              (void) fprintf(image->file,"\n");
                              count=0;
                            };
                        };
                      if (image->previous == (Image *) NULL)
                        if (QuantumTick(y,image->rows))
                          ProgressMonitor(SaveImageText,y,image->rows);
                      bit=0;
                      byte=0;
                      x=0;
                      y++;
                    }
                  }
                p++;
              }
            }
          if (count != 0)
            (void) fprintf(image->file,"\n");
        }
      else
        {
          /*
            Dump PseudoClass image.
          */
          (void) fprintf(image->file,"%u %u\n%d\n%d\n0\n",
            image->columns,image->rows,(int) (image->class == PseudoClass),
            (int) (image_info->compression == NoCompression));
          /*
            Dump number of colors and colormap.
          */
          (void) fprintf(image->file,"%u\n",image->colors);
          for (i=0; i < (int) image->colors; i++)
            (void) fprintf(image->file,"%02lx%02lx%02lx\n",
              DownScale(image->colormap[i].red),
              DownScale(image->colormap[i].green),
              DownScale(image->colormap[i].blue));
          switch (image_info->compression)
          {
            case RunlengthEncodedCompression:
            default:
            {
              /*
                Dump runlength-encoded PseudoColor packets.
              */
              for (i=0; i < (int) image->packets; i++)
              {
                for (length=p->length; length >= 0; length-=256)
                {
                  (void) fprintf(image->file,"%02x%02x",(unsigned int) p->index,
                    (unsigned int) Min(length,0xff));
                  x++;
                  if (x == 18)
                    {
                      x=0;
                      (void) fprintf(image->file,"\n");
                    }
                }
                p++;
                if (image->previous == (Image *) NULL)
                  if (QuantumTick(i,image->packets))
                    ProgressMonitor(SaveImageText,i,image->packets);
              }
              break;
            }
            case NoCompression:
            {
              /*
                Dump uncompressed PseudoColor packets.
              */
              for (i=0; i < (int) image->packets; i++)
              {
                for (j=0; j <= ((int) p->length); j++)
                {
                  (void) fprintf(image->file,"%02x",(unsigned int) p->index);
                  x++;
                  if (x == 36)
                    {
                      x=0;
                      (void) fprintf(image->file,"\n");
                    }
                }
                p++;
                if (image->previous == (Image *) NULL)
                  if (QuantumTick(i,image->packets))
                    ProgressMonitor(SaveImageText,i,image->packets);
              }
              break;
            }
          }
          (void) fprintf(image->file,"\n");
        }
    (void) fprintf(image->file,"%%%%EndData\n");
    if (Latin1Compare(image_info->magick,"PS") != 0)
      (void) fprintf(image->file,"end\n");
    (void) fprintf(image->file,"%%%%PageTrailer\n");
    if (image->next == (Image *) NULL)
      break;
    image->next->file=image->file;
    image=image->next;
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  (void) fprintf(image->file,"%%%%Trailer\n");
  if (page > 1)
    (void) fprintf(image->file,"%%%%BoundingBox: %g %g %g %g\n",
      bounding_box.x1,bounding_box.y1,bounding_box.x2,bounding_box.y2);
  (void) fprintf(image->file,"%%%%EOF\n");
  CloseImage(image);
  return(True);
}
