/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   SSSSS  DDDD                               %
%                            P   P  SS     D   D                              %
%                            PPPP    SSS   D   D                              %
%                            P         SS  D   D                              %
%                            P      SSSSS  DDDD                               %
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
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a c k b i t s D e c o d e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method PackbitsDecodeImage uncompresses an image via Macintosh Packbits
%  encoding specific to the Adobe Photoshop image format.
%
%  The format of the PackbitsDecodeImage routine is:
%
%      status=PackbitsDecodeImage(image,channel)
%
%  A description of each parameter follows:
%
%    o status: Method PackbitsDecodeImage return True if the image is
%      decoded.  False is returned if there is an error occurs.
%
%    o image: The address of a structure of type Image.
%
%    o channel:  Specifies which channel: red, green, blue, or index to
%      decode the pixel values into.
%
%
*/
static unsigned int PackbitsDecodeImage(Image *image,const int channel)
{
  int
    count,
    pixel;

  long
    length;

  register int
    i;

  register RunlengthPacket
    *q;

  q=image->pixels;
  length=image->columns*image->rows;
  while (length > 0)
  {
    count=fgetc(image->file);
    if (count >= 128)
      count-=256;
    if (count < 0)
      {
        if (count == -128)
          continue;
        count=(-count+1);
        pixel=fgetc(image->file);
        for ( ; count > 0; count--)
        {
          switch (channel)
          {
            case 0:
            {
              q->red=(Quantum) pixel;
              if (image->class == PseudoClass)
                q->index=(unsigned short) pixel;
              break;
            }
            case 1:
            {
              q->green=(Quantum) pixel;
              break;
            }
            case 2:
            {
              q->blue=(Quantum) pixel;
              break;
            }
            case 3:
            default:
            {
              q->index=(unsigned short) pixel;
              break;
            }
          }
          q->length=0;
          q++;
          length--;
        }
        continue;
      }
    count++;
    for (i=count; i > 0; i--)
    {
      pixel=fgetc(image->file);
      switch (channel)
      {
        case 0:
        {
          q->red=(Quantum) pixel;
          if (image->class == PseudoClass)
            q->index=(unsigned short) pixel;
          break;
        }
        case 1:
        {
          q->green=(Quantum) pixel;
          break;
        }
        case 2:
        {
          q->blue=(Quantum) pixel;
          break;
        }
        case 3:
        default:
        {
          q->index=(unsigned short) pixel;
          break;
        }
      }
      q->length=0;
      q++;
      length--;
    }
  }
  /*
    Guarentee the correct number of pixel packets.
  */
  if (length > 0)
    {
      MagickWarning(CorruptImageWarning,"insufficient image data in file",
        image->filename);
      return(False);
    }
  else
    if (length < 0)
      {
        MagickWarning(CorruptImageWarning,"too much image data in file",
          image->filename);
        return(False);
      }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P S D I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPSDImage reads an Adobe Photoshop image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPSDImage routine is:
%
%      image=ReadPSDImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPSDImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadPSDImage(const ImageInfo *image_info)
{
#define BitmapMode  0
#define GrayscaleMode  1
#define IndexedMode  2
#define CMYKMode  4

  typedef struct _ChannelInfo
  {
    short int
      type;

    unsigned long
      size;
  } ChannelInfo;

  typedef struct _LayerInfo
  {
    unsigned int
      width,
      height;

    int
      x,
      y;

    unsigned short
      channels;

    ChannelInfo
      channel_info[24];

    char
      blendkey[4];

    unsigned char
      opacity,
      clipping,
      flags;

    Image
      *image;
  } LayerInfo;

  typedef struct _PSDHeader
  {
    char
      signature[4];

    unsigned short
      channels,
      version;

    unsigned char
      reserved[6];

    unsigned int
      rows,
      columns;

    unsigned short
      depth,
      mode;
  } PSDHeader;

  char
    type[4];

  Image
    *image;

  LayerInfo
    *layer_info;

  long
    count,
    length,
    size;

  PSDHeader
    psd_header;

  register int
    i,
    j,
    k;

  register RunlengthPacket
    *q;

  short int
    number_layers;

  unsigned int
    status;

  unsigned short
    compression;

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
    Read image header.
  */
  status=ReadData((char *) psd_header.signature,1,4,image->file);
  psd_header.version=MSBFirstReadShort(image->file);
  if ((status == False) || (strncmp(psd_header.signature,"8BPS",4) != 0) ||
      (psd_header.version != 1))
    ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
  (void) ReadData((char *) psd_header.reserved,1,6,image->file);
  psd_header.channels=MSBFirstReadShort(image->file);
  psd_header.rows=MSBFirstReadLong(image->file);
  psd_header.columns=MSBFirstReadLong(image->file);
  psd_header.depth=MSBFirstReadShort(image->file);
  psd_header.mode=MSBFirstReadShort(image->file);
  /*
    Initialize image.
  */
  if (psd_header.mode == CMYKMode)
    image->colorspace=CMYKColorspace;
  else
    image->matte=psd_header.channels >= 4;
  image->columns=psd_header.columns;
  image->rows=psd_header.rows;
  image->depth=Max(psd_header.depth,QuantumDepth);
  length=MSBFirstReadLong(image->file);
  if ((psd_header.mode == BitmapMode) ||
      (psd_header.mode == GrayscaleMode) ||
      (psd_header.mode == IndexedMode) || (length > 0))
    {
      /*
        Create colormap.
      */
      image->class=PseudoClass;
      image->colors=1 << image->depth;
      image->colormap=(ColorPacket *)
        AllocateMemory(image->colors*sizeof(ColorPacket));
      if (image->colormap == (ColorPacket *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=(MaxRGB*i)/(image->colors-1);
        image->colormap[i].green=(MaxRGB*i)/(image->colors-1);
        image->colormap[i].blue=(MaxRGB*i)/(image->colors-1);
      }
      if (length > 0)
        {
          /*
            Read PSD raster colormap.
          */
          for (i=0; i < (int) image->colors; i++)
            image->colormap[i].red=fgetc(image->file);
          for (i=0; i < (int) image->colors; i++)
            image->colormap[i].green=fgetc(image->file);
          for (i=0; i < (int) image->colors; i++)
            image->colormap[i].blue=fgetc(image->file);
        }
    }
  length=MSBFirstReadLong(image->file);
  while (length > 0)
  {
    /*
      Read image resource block.
    */
    status=ReadData((char *) type,1,4,image->file);
    if ((status == False) || (strncmp(type,"8BIM",4) != 0))
      ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
    (void) MSBFirstReadShort(image->file);
    count=fgetc(image->file);
    if (count > 0)
      for (i=0; i < count; i++)
        (void) fgetc(image->file);
    if (!(count & 0x01))
      {
        (void) fgetc(image->file);
        length--;
      }
    size=MSBFirstReadLong(image->file);
    for (i=0; i < size; i++)
      (void) fgetc(image->file);
    length-=(count+size+11);
    if (size & 0x01)
      {
        (void) fgetc(image->file);
        length--;
      }
  }
  if (image_info->ping)
    {
      CloseImage(image);
      return(image);
    }
  layer_info=(LayerInfo *) NULL;
  number_layers=0;
  length=MSBFirstReadLong(image->file);
  if (length > 0)
    {
      /*
        Read layer and mask block.
      */
      size=MSBFirstReadLong(image->file);
      number_layers=MSBFirstReadShort(image->file);
      number_layers=AbsoluteValue(number_layers);
      layer_info=(LayerInfo *) AllocateMemory(number_layers*sizeof(LayerInfo));
      if (layer_info == (LayerInfo *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < number_layers; i++)
      {
        layer_info[i].y=MSBFirstReadLong(image->file);
        layer_info[i].x=MSBFirstReadLong(image->file);
        layer_info[i].height=MSBFirstReadLong(image->file)-layer_info[i].y;
        layer_info[i].width=MSBFirstReadLong(image->file)-layer_info[i].x;
        layer_info[i].channels=MSBFirstReadShort(image->file);
        if (layer_info[i].channels > 24)
          ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
        for (j=0; j < (int) layer_info[i].channels; j++)
        {
          layer_info[i].channel_info[j].type=MSBFirstReadShort(image->file);
          layer_info[i].channel_info[j].size=MSBFirstReadLong(image->file);
        }
        status=ReadData((char *) type,1,4,image->file);
        if ((status == False) || (strncmp(type,"8BIM",4) != 0))
          ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
        (void) ReadData((char *) layer_info[i].blendkey,1,4,image->file);
        layer_info[i].opacity=fgetc(image->file);
        layer_info[i].clipping=fgetc(image->file);
        layer_info[i].flags=fgetc(image->file);
        (void) fgetc(image->file);  /* filler */
        size=MSBFirstReadLong(image->file);
        for (j=0; j < size; j++)
          (void) fgetc(image->file);
        /*
          Allocate layered image.
        */
        layer_info[i].image=AllocateImage(image_info);
        if (layer_info[i].image == (Image *) NULL)
          {
            for (j=0; j < i; j++)
              DestroyImage(layer_info[j].image);
            ReaderExit(ResourceLimitWarning,"Memory allocation failed",
              image);
          }
        layer_info[i].image->file=image->file;
        layer_info[i].image->class=image->class;
        if (psd_header.mode == CMYKMode)
          layer_info[i].image->colorspace=CMYKColorspace;
        else
          layer_info[i].image->matte=layer_info[i].channels >= 4;
        if (image->colormap != (ColorPacket *) NULL)
          {
            /*
              Convert pixels to Runlength encoded.
            */
            layer_info[i].image->colormap=(ColorPacket *)
              AllocateMemory(image->colors*sizeof(ColorPacket));
            if (layer_info[i].image->colormap == (ColorPacket *) NULL)
              {
                for (j=0; j < i; j++)
                  DestroyImage(layer_info[j].image);
                ReaderExit(ResourceLimitWarning,"Memory allocation failed",
                  image);
              }
          }
        layer_info[i].image->columns=layer_info[i].width;
        layer_info[i].image->rows=layer_info[i].height;
        layer_info[i].image->packets=
          layer_info[i].image->columns*layer_info[i].image->rows;
        layer_info[i].image->pixels=(RunlengthPacket *) AllocateMemory(
          (layer_info[i].image->packets+256)*sizeof(RunlengthPacket));
        if (layer_info[i].image->pixels == (RunlengthPacket *) NULL)
          {
            for (j=0; j < i; j++)
              DestroyImage(layer_info[j].image);
            ReaderExit(ResourceLimitWarning,"Memory allocation failed",
              image);
          }
        SetImage(layer_info[i].image);
      }
      /*
        Read pixel data for each layer.
      */
      for (i=0; i < number_layers; i++)
      {
        for (j=0; j < (int) layer_info[i].channels; j++)
        {
          compression=MSBFirstReadShort(layer_info[i].image->file);
          if (compression != 0)
            {
              for (k=0; k < (int) layer_info[i].image->rows; k++)
                (void) MSBFirstReadShort(image->file);
              (void) PackbitsDecodeImage(layer_info[i].image,
                layer_info[i].channel_info[j].type);
            }
          else
            {
              /*
                Read uncompressed pixel data as separate planes.
              */
              q=layer_info[i].image->pixels;
              for (k=0; k < (int) layer_info[i].image->packets; k++)
              {
                switch (layer_info[i].channel_info[j].type)
                {
                  case 0:
                  {
                    ReadQuantumFile(q->red);
                    q->index=q->red;
                    break;
                  }
                  case 1:
                  {
                    ReadQuantumFile(q->green);
                    break;
                  }
                  case 2:
                  {
                    ReadQuantumFile(q->blue);
                    break;
                  }
                  case 3:
                  default:
                  {
                    ReadQuantumFile(q->index);
                    break;
                  }
                }
                q->length=0;
                q++;
              }
            }
        }
        if (layer_info[i].image->class == PseudoClass)
          SyncImage(layer_info[i].image);
        else
          if (layer_info[i].image->colorspace == CMYKColorspace)
            {
              /*
                Correct CMYK levels.
              */
              q=layer_info[i].image->pixels;
              for (k=0; k < (int) layer_info[i].image->packets; k++)
              {
                q->red=MaxRGB-q->red;
                q->green=MaxRGB-q->green;
                q->blue=MaxRGB-q->blue;
                q->index=MaxRGB-q->index;
                q++;
              }
            }
          else
            if (layer_info[i].opacity != Opaque)
              {
                /*
                  Correct for opacity level.
                */
                q=layer_info[i].image->pixels;
                for (k=0; k < (int) layer_info[i].image->packets; k++)
                {
                  q->index=(int) (q->index*layer_info[i].opacity)/Opaque;
                  q++;
                }
              }
        layer_info[i].image->file=(FILE *) NULL;
      }
      for (i=0; i < 4; i++)
        (void) fgetc(image->file);
    }
  /*
    Convert pixels to Runlength encoded.
  */
  compression=MSBFirstReadShort(image->file);
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    AllocateMemory((image->packets+256)*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  SetImage(image);
  if (compression != 0)
    {
      /*
        Read Packbit encoded pixel data as separate planes.
      */
      for (i=0; i < (int) (image->rows*psd_header.channels); i++)
        (void) MSBFirstReadShort(image->file);
      for (i=0; i < (int) psd_header.channels; i++)
        (void) PackbitsDecodeImage(image,i);
    }
  else
    {
      /*
        Read uncompressed pixel data as separate planes.
      */
      for (i=0; i < (int) psd_header.channels; i++)
      {
        q=image->pixels;
        for (j=0; j < (int) image->packets; j++)
        {
          switch (i)
          {
            case 0:
            {
              ReadQuantumFile(q->red);
              q->index=q->red;
              break;
            }
            case 1:
            {
              ReadQuantumFile(q->green);
              break;
            }
            case 2:
            {
              ReadQuantumFile(q->blue);
              break;
            }
            case 3:
            default:
            {
              ReadQuantumFile(q->index);
              break;
            }
          }
          q->length=0;
          q++;
        }
      }
    }
  if (image->class == PseudoClass)
    SyncImage(image);
  else
    if (image->colorspace == CMYKColorspace)
      {
        /*
          Correct CMYK levels.
        */
        q=image->pixels;
        for (i=0; i < (int) image->packets; i++)
        {
          q->red=MaxRGB-q->red;
          q->green=MaxRGB-q->green;
          q->blue=MaxRGB-q->blue;
          q->index=MaxRGB-q->index;
          q++;
        }
      }
  for (i=0; i < number_layers; i++)
  {
    /*
      Composite layer onto image.
    */
    if ((layer_info[i].width != 0) && (layer_info[i].height != 0))
      CompositeImage(image,OverCompositeOp,layer_info[i].image,
        layer_info[i].x,layer_info[i].y);
    layer_info[i].image->colormap=(ColorPacket *) NULL;
    DestroyImage(layer_info[i].image);
  }
  image->matte=False;
  if (image->colorspace != CMYKColorspace)
    image->matte=psd_header.channels >= 4;
  CondenseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WritePSDImage writes an image in the Adobe Photoshop encoded image
%  format.
%
%  The format of the WritePSDImage routine is:
%
%      status=WritePSDImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Method WritePSDImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WritePSDImage(const ImageInfo *image_info,Image *image)
{
  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image_info,image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  (void) fwrite("8BPS",1,4,image->file);
  MSBFirstWriteShort(1,image->file);  /* version */
  (void) fwrite("      ",1,6,image->file);  /* reserved */
  if (image->class == PseudoClass)
    MSBFirstWriteShort(1,image->file);
  else
    MSBFirstWriteShort(image->matte ? 4 : 3,image->file);
  MSBFirstWriteLong(image->rows,image->file);
  MSBFirstWriteLong(image->columns,image->file);
  MSBFirstWriteShort(image->depth,image->file);
  if (((image_info->colorspace != UndefinedColorspace) ||
       (image->colorspace != CMYKColorspace)) &&
       (image_info->colorspace != CMYKColorspace))
    {
      TransformRGBImage(image,RGBColorspace);
      MSBFirstWriteShort(image->class == PseudoClass ? 2 : 3,image->file);
    }
  else
    {
      if (image->colorspace != CMYKColorspace)
        RGBTransformImage(image,CMYKColorspace);
      MSBFirstWriteShort(4,image->file);
    }
  if ((image->class == DirectClass) || (image->colors > 256))
    MSBFirstWriteLong(0,image->file);
  else
    {
      /*
        Write PSD raster colormap.
      */
      MSBFirstWriteLong(768,image->file);
      for (i=0; i < (int) image->colors; i++)
        (void) fputc(DownScale(image->colormap[i].red),image->file);
      for ( ; i < 256; i++)
        (void) fputc(0,image->file);
      for (i=0; i < (int) image->colors; i++)
        (void) fputc(DownScale(image->colormap[i].green),image->file);
      for ( ; i < 256; i++)
        (void) fputc(0,image->file);
      for (i=0; i < (int) image->colors; i++)
        (void) fputc(DownScale(image->colormap[i].blue),image->file);
      for ( ; i < 256; i++)
        (void) fputc(0,image->file);
    }
  MSBFirstWriteLong(0,image->file);  /* image resource block */
  MSBFirstWriteLong(0,image->file);  /* layer and mask block */
  MSBFirstWriteShort(0,image->file);  /* compression */
  /*
    Write uncompressed pixel data as separate planes.
  */
  p=image->pixels;
  if (image->class == PseudoClass)
    for (i=0; i < (int) image->packets; i++)
    {
      for (j=0; j <= ((int) p->length); j++)
        WriteQuantumFile(p->index);
      p++;
    }
  else
    {
      for (i=0; i < (int) image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
          if (image->colorspace != CMYKColorspace)
            WriteQuantumFile(p->red)
          else
            WriteQuantumFile(MaxRGB-p->red);
        p++;
      }
      p=image->pixels;
      for (i=0; i < (int) image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
          if (image->colorspace != CMYKColorspace)
            WriteQuantumFile(p->green)
          else
            WriteQuantumFile(MaxRGB-p->green);
        p++;
      }
      p=image->pixels;
      for (i=0; i < (int) image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
          if (image->colorspace != CMYKColorspace)
            WriteQuantumFile(p->blue)
          else
            WriteQuantumFile(MaxRGB-p->blue);
        p++;
      }
      p=image->pixels;
      if (image->matte || (image->colorspace == CMYKColorspace))
        for (i=0; i < (int) image->packets; i++)
        {
          for (j=0; j <= ((int) p->length); j++)
            if (image->colorspace != CMYKColorspace)
              WriteQuantumFile(p->index)
            else
              WriteQuantumFile(MaxRGB-p->index);
          p++;
        }
    }
  CloseImage(image);
  return(True);
}
