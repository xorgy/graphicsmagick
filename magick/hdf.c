/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            H   H  DDDD   FFFFF                              %
%                            H   H  D   D  F                                  %
%                            HHHHH  D   D  FFF                                %
%                            H   H  D   D  F                                  %
%                            H   H  DDDD   F                                  %
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
%  Copyright (C) 2000 ImageMagick Studio, a non-profit organization dedicated %
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
%
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"
#if defined(HasHDF)
#include "hdf.h"
#undef BSD
#undef LOCAL
#endif

/*
  Forward declarations.
*/
static unsigned int
  WriteHDFImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H D F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsHDF returns True if the image format type, identified by the
%  magick string, is HDF.
%
%  The format of the IsHDF method is:
%
%      unsigned int IsHDF(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsHDF returns True if the image format type is HDF.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static unsigned int IsHDF(const unsigned char *magick,const unsigned int length)
{
  if (length < 4)
    return(False);
  if (strncmp((char *) magick,"\016\003\023\001",4) == 0)
    return(True);
  return(False);
}

#if defined(HasHDF)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d H D F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadHDFImage reads a Hierarchical Data Format image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadHDFImage method is:
%
%      Image *ReadHDFImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadHDFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadHDFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  ClassType
    class;

  Image
    *image;

  int
    interlace,
    is_palette,
    status,
    y;

  int32
    height,
    length,
    width;

  register int
    i,
    x;

  register unsigned char
    *p;

  register PixelPacket
    *q;

  uint16
    reference;

  unsigned char
    *hdf_pixels;

  unsigned int
    packet_size;

  /*
    Open image file.
  */
  image=AllocateImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryType);
  if (status == False)
    ThrowReaderException(FileOpenWarning,"Unable to open file",image);
  /*
    Read HDF image.
  */
  class=DirectClass;
  status=DF24getdims(image->filename,&width,&height,&interlace);
  if (status == -1)
    {
      class=PseudoClass;
      status=DFR8getdims(image->filename,&width,&height,&is_palette);
    }
  if (status == -1)
    ThrowReaderException(CorruptImageWarning,
      "Image file or does not contain any image data",image);
  do
  {
    /*
      Initialize image structure.
    */
    image->class=class;
    image->columns=width;
    image->rows=height;
    image->depth=8;
    if (image->class == PseudoClass)
      image->colors=256;
    if (image_info->ping)
      {
        CloseBlob(image);
        return(image);
      }
    packet_size=image->class == DirectClass ? 3 : 1;
    hdf_pixels=(unsigned char *)
      AllocateMemory(packet_size*image->columns*image->rows);
    if (hdf_pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitWarning,"Memory allocation failed",image);
    if (image->class == PseudoClass)
      {
        unsigned char
          *hdf_palette;

        /*
          Create colormap.
        */
        hdf_palette=(unsigned char *) AllocateMemory(768);
        image->colormap=(PixelPacket *)
          AllocateMemory(image->colors*sizeof(PixelPacket));
        if ((hdf_palette == (unsigned char *) NULL) ||
            (image->colormap == (PixelPacket *) NULL))
          ThrowReaderException(ResourceLimitWarning,"Memory allocation failed",image);
        (void) DFR8getimage(image->filename,hdf_pixels,(int) image->columns,
          (int) image->rows,hdf_palette);
        reference=DFR8lastref();
        /*
          Convert HDF raster image to PseudoClass pixel packets.
        */
        p=hdf_palette;
        if (is_palette)
          for (i=0; i < 256; i++)
          {
            image->colormap[i].red=UpScale(*p++);
            image->colormap[i].green=UpScale(*p++);
            image->colormap[i].blue=UpScale(*p++);
          }
        else
          for (i=0; i < (int) image->colors; i++)
          {
            image->colormap[i].red=(Quantum) UpScale(i);
            image->colormap[i].green=(Quantum) UpScale(i);
            image->colormap[i].blue=(Quantum) UpScale(i);
          }
        FreeMemory(hdf_palette);
        p=hdf_pixels;
        for (y=0; y < (int) image->rows; y++)
        {
          if (!SetPixelCache(image,0,y,image->columns,1))
            break;
          for (x=0; x < (int) image->columns; x++)
            image->indexes[x]=(*p++);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
      }
    else
      {
        /*
          Convert HDF raster image to DirectClass pixel packets.
        */
        (void) DF24getimage(image->filename,(void *) hdf_pixels,image->columns,
          image->rows);
        reference=DF24lastref();
        p=hdf_pixels;
        image->interlace=interlace ? PlaneInterlace : NoInterlace;
        for (y=0; y < (int) image->rows; y++)
        {
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            q->red=UpScale(*p++);
            q->green=UpScale(*p++);
            q->blue=UpScale(*p++);
            q++;
          }
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
      }
    length=DFANgetlablen(image->filename,DFTAG_RIG,reference);
    if (length > 0)
      {
        char
          *label;

        /*
          Read the image label.
        */
        length+=MaxTextExtent;
        label=(char *) AllocateMemory(length);
        if (label != (char *) NULL)
          {
            DFANgetlabel(image->filename,DFTAG_RIG,reference,label,length);
            (void) SetImageAttribute(image,"Label",label);
            FreeMemory(label);
          }
      }
    length=DFANgetdesclen(image->filename,DFTAG_RIG,reference);
    if (length > 0)
      {
        char
          *comments;

        /*
          Read the image comments.
        */
        length+=MaxTextExtent;
        comments=(char *) AllocateMemory(length);
        if (comments != (char *) NULL)
          {
            DFANgetdesc(image->filename,DFTAG_RIG,reference,comments,length);
            (void) SetImageAttribute(image,"Comment",comments);
            FreeMemory(comments);
          }
      }
    FreeMemory(hdf_pixels);
    if (image->class == PseudoClass)
      SyncImage(image);
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    class=DirectClass;
    status=DF24getdims(image->filename,&width,&height,&interlace);
    if (status == -1)
      {
        class=PseudoClass;
        status=DFR8getdims(image->filename,&width,&height,&is_palette);
      }
    if (status != -1)
      {
        /*
          Allocate next image structure.
        */
        AllocateNextImage(image_info,image);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        image=image->next;
        ProgressMonitor(LoadImagesText,TellBlob(image),image->filesize);
      }
  } while (status != -1);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseBlob(image);
  return(image);
}
#else
static Image *ReadHDFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  ThrowException(exception,MissingDelegateWarning,
    "HDF library is not available",image_info->filename);
  return((Image *) NULL);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H D F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterHDFImage adds attributes for the HDF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterHDFImage method is:
%
%      RegisterHDFImage(void)
%
*/
Export void RegisterHDFImage(void)
{
  MagickInfo
    *entry;

#if defined(HasHDF)
  entry=SetMagickInfo("HDF");
  entry->decoder=ReadHDFImage;
  entry->encoder=WriteHDFImage;
  entry->magick=IsHDF;
  entry->blob_support=False;
  entry->description=AllocateString("Hierarchical Data Format");
  RegisterMagickInfo(entry);
#endif
}

#if defined(HasHDF)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H D F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteHDFImage writes an image in the Hierarchial Data Format image
%  format.
%
%  The format of the WriteHDFImage method is:
%
%      unsigned int WriteHDFImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteHDFImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteHDFImage(const ImageInfo *image_info,Image *image)
{
  ImageAttribute
    *attribute;

  int
    status,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p;

  register unsigned char
    *q;

  uint16
    reference;

  unsigned char
    *hdf_pixels;

  unsigned short
    compression;

  unsigned int
    packet_size,
    scene;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    ThrowWriterException(FileOpenWarning,"Unable to open file",image);
  CloseBlob(image);
  scene=0;
  do
  {
    /*
      Initialize raster file header.
    */
    TransformRGBImage(image,RGBColorspace);
    packet_size=image->class == DirectClass ? 3 : 11;
    hdf_pixels=(unsigned char *)
      AllocateMemory(packet_size*image->columns*image->rows);
    if (hdf_pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitWarning,"Memory allocation failed",image);
    if (!IsPseudoClass(image) && !IsGrayImage(image))
      {
        /*
          Convert DirectClass packet to HDF pixels.
        */
        q=hdf_pixels;
        switch (image_info->interlace)
        {
          case NoInterlace:
          default:
          {
            /*
              No interlacing:  RGBRGBRGBRGBRGBRGB...
            */
            DF24setil(DFIL_PIXEL);
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=DownScale(p->red);
                *q++=DownScale(p->green);
                *q++=DownScale(p->blue);
                p++;
              }
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  ProgressMonitor(SaveImageText,y,image->rows);
            }
            break;
          }
          case LineInterlace:
          {
            /*
              Line interlacing:  RRR...GGG...BBB...RRR...GGG...BBB...
            */
            DF24setil(DFIL_LINE);
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=DownScale(p->red);
                p++;
              }
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=DownScale(p->green);
                p++;
              }
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=DownScale(p->blue);
                p++;
              }
              if (QuantumTick(y,image->rows))
                ProgressMonitor(SaveImageText,y,image->rows);
            }
            break;
          }
          case PlaneInterlace:
          case PartitionInterlace:
          {
            /*
              Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
            */
            DF24setil(DFIL_PLANE);
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=DownScale(p->red);
                p++;
              }
            }
            ProgressMonitor(SaveImageText,100,400);
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=DownScale(p->green);
                p++;
              }
            }
            ProgressMonitor(SaveImageText,250,400);
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=DownScale(p->blue);
                p++;
              }
            }
            ProgressMonitor(SaveImageText,400,400);
            break;
          }
        }
        if (scene == 0)
          status=DF24putimage(image->filename,(void *) hdf_pixels,
            image->columns,image->rows);
        else
          status=DF24addimage(image->filename,(void *) hdf_pixels,
            image->columns,image->rows);
        reference=DF24lastref();
      }
    else
      {
        /*
          Convert PseudoClass packet to HDF pixels.
        */
        q=hdf_pixels;
        if (IsGrayImage(image))
          for (y=0; y < (int) image->rows; y++)
          {
            p=GetPixelCache(image,0,y,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            for (x=0; x < (int) image->columns; x++)
            {
              *q++=DownScale(Intensity(*p));
              p++;
            }
            if (image->previous == (Image *) NULL)
              if (QuantumTick(y,image->rows))
                ProgressMonitor(SaveImageText,y,image->rows);
          }
        else
          {
            unsigned char
              *hdf_palette;

            hdf_palette=(unsigned char *) AllocateMemory(768);
            if (hdf_palette == (unsigned char *) NULL)
              ThrowWriterException(ResourceLimitWarning,"Memory allocation failed",image);
            q=hdf_palette;
            for (i=0; i < (int) image->colors; i++)
            {
              *q++=DownScale(image->colormap[i].red);
              *q++=DownScale(image->colormap[i].green);
              *q++=DownScale(image->colormap[i].blue);
            }
            (void) DFR8setpalette(hdf_palette);
            FreeMemory(hdf_palette);
            q=hdf_pixels;
            for (y=0; y < (int) image->rows; y++)
            {
              if (!GetPixelCache(image,0,y,image->columns,1))
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=image->indexes[x];
                p++;
              }
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  ProgressMonitor(SaveImageText,y,image->rows);
            }
          }
        compression=image_info->compression == NoCompression ? 0 : DFTAG_RLE;
        if (scene == 0)
          status=DFR8putimage(image->filename,(void *) hdf_pixels,
            image->columns,image->rows,compression);
        else
          status=DFR8addimage(image->filename,(void *) hdf_pixels,
            image->columns,image->rows,compression);
        reference=DFR8lastref();
      }
    attribute=GetImageAttribute(image,"Label");
    if (attribute != (ImageAttribute *) NULL)
      (void) DFANputlabel(image->filename,DFTAG_RIG,reference,attribute->value);
    attribute=GetImageAttribute(image,"Comment");
    if (attribute != (ImageAttribute *) NULL)
      (void) DFANputdesc(image->filename,DFTAG_RIG,reference,attribute->value,
        Extent(attribute->value)+1);
    FreeMemory(hdf_pixels);
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  return(status != -1);
}
#else
static unsigned int WriteHDFImage(const ImageInfo *image_info,Image *image)
{
  ThrowBinaryException(MissingDelegateWarning,"HDF library is not available",
    image->filename);
}
#endif
