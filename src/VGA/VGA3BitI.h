/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 2.0
	https://creativecommons.org/licenses/by-sa/2.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "VGA.h"
#include "../Graphics/GraphicsR1G1B1A1.h"

class VGA3BitI : public VGA, public GraphicsR1G1B1A1
{
  public:
	VGA3BitI(const int i2sIndex = 1)
		: VGA(i2sIndex)
	{
	}

	bool init(const int *mode, const int RPin, const int GPin, const int BPin, const int hsyncPin, const int vsyncPin)
	{
		int pinMap[24] = {
			-1, -1, -1, -1, -1, -1, -1, -1,
			RPin,
			GPin,
			BPin,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			hsyncPin, vsyncPin};
		return VGA::init(mode, pinMap);
	}

	virtual float pixelAspect() const
	{
		return float(vdivider) / hdivider;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

  protected:
	virtual bool useInterrupt()
	{ 
		return true; 
	};

	virtual void interrupt()
	{
		unsigned long *signal = (unsigned long*)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer();
		unsigned long *pixels = &((unsigned long*)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer())[(hfront + hsync + hback) / 2];
		unsigned long base, baseh;
		if (currentLine >= vfront && currentLine < vfront + vsync)
		{
			baseh = (vsyncBit | hsyncBit) | ((vsyncBit | hsyncBit) << 16);
			base = (vsyncBit | hsyncBitI) | ((vsyncBit | hsyncBitI) << 16);
		}
		else
		{
			baseh = (vsyncBitI | hsyncBit) | ((vsyncBitI | hsyncBit) << 16);
			base = (vsyncBitI | hsyncBitI) | ((vsyncBitI | hsyncBitI) << 16);
		}
		for (int i = 0; i < hfront / 2; i++)
			signal[i] = base;
		for (int i = hfront / 2; i < (hfront + hsync) / 2; i++)
			signal[i] = baseh;
		for (int i = (hfront + hsync) / 2; i < (hfront + hsync + hback) / 2; i++)
			signal[i] = base;

		int y = (currentLine - vfront - vsync - vback) / vdivider;
		if (y >= 0 && y < yres)
		{
			unsigned char *line = frontBuffer[y];
			for (int i = 0; i < xres / 2; i++)
			{
				//writing two pixels improves speed drastically (avoids reading in higher word)
				pixels[i] = base | ((line[i] >> 4) & 7) | ((line[i] & 7) << 16);
			}
		}
		else
			for (int i = 0; i < xres / 2; i++)
			{
				pixels[i] = base | (base << 16);
			}
		currentLine = (currentLine + 1) % totalLines;
		dmaBufferDescriptorActive = (dmaBufferDescriptorActive + 1) % dmaBufferDescriptorCount;
	}
};
