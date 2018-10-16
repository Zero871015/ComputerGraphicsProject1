#include "Application.h"
#include "qt_opengl_framework.h"
#include <vector>

Application::Application()
{

}
Application::~Application()
{

}
//****************************************************************************
//
// * 初始畫面，並顯示Ntust.png圖檔
// 
//============================================================================
void Application::createScene( void )
{
	
	ui_instance = Qt_Opengl_Framework::getInstance();
	
}

//****************************************************************************
//
// * 打開指定圖檔
// 
//============================================================================
void Application::openImage( QString filePath )
{
	mImageSrc.load(filePath);
	mImageDst.load(filePath);

	renew();

	img_data = mImageSrc.bits();
	img_width = mImageSrc.width();
	img_height = mImageSrc.height();

	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
}
//****************************************************************************
//
// * 刷新畫面
// 
//============================================================================
void Application::renew()
{
	ui_instance = Qt_Opengl_Framework::getInstance();

	ui_instance->ui.label->clear();
	ui_instance->ui.label->setPixmap(QPixmap::fromImage(mImageDst));

	std::cout << "Renew" << std::endl;
}

//****************************************************************************
//
// * 畫面初始化
// 
//============================================================================
void Application::reload()
{
	ui_instance = Qt_Opengl_Framework::getInstance();

	ui_instance->ui.label->clear();
	ui_instance->ui.label->setPixmap(QPixmap::fromImage(mImageSrc));
}

//****************************************************************************
//
// * 儲存圖檔
// 
//============================================================================
void Application::saveImage(QString filePath )
{
	mImageDst.save(filePath);
}

//****************************************************************************
//
// * 將圖檔資料轉換為RGB色彩資料
// 
//============================================================================
unsigned char* Application::To_RGB( void )
{
	unsigned char *rgb = new unsigned char[img_width * img_height * 3];
	int i, j;

	if (! img_data )
		return NULL;

	// Divide out the alpha
	for (i = 0; i < img_height; i++)
	{
		int in_offset = i * img_width * 4;
		int out_offset = i * img_width * 3;

		for (j = 0 ; j < img_width ; j++)
		{
			RGBA_To_RGB(img_data + (in_offset + j*4), rgb + (out_offset + j*3));
		}
	}

	return rgb;
}

void Application::RGBA_To_RGB( unsigned char *rgba, unsigned char *rgb )
{
	const unsigned char	BACKGROUND[3] = { 0, 0, 0 };

	unsigned char  alpha = rgba[3];

	if (alpha == 0)
	{
		rgb[0] = BACKGROUND[0];
		rgb[1] = BACKGROUND[1];
		rgb[2] = BACKGROUND[2];
	}
	else
	{
		float	alpha_scale = (float)255 / (float)alpha;
		int	val;
		int	i;

		for (i = 0 ; i < 3 ; i++)
		{
			val = (int)floor(rgba[i] * alpha_scale);
			if (val < 0)
				rgb[i] = 0;
			else if (val > 255)
				rgb[i] = 255;
			else
				rgb[i] = val;
		}
	}
}
//------------------------Color------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Convert image to grayscale.  Red, green, and blue channels should all 
//  contain grayscale value.  Alpha channel shoould be left unchanged.  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Gray()
{
	unsigned char *rgb = To_RGB();

	for (int i=0; i<img_height; i++)
	{
		for (int j=0; j<img_width; j++)
		{
			int offset_rgb = i*img_width*3+j*3;
			int offset_rgba = i*img_width*4+j*4;
			unsigned char gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];

			for (int k=0; k<3; k++)
				img_data[offset_rgba+k] = gray;
			img_data[offset_rgba + aa] = WHITE;
		}
	}
	
	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using uniform quantization.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Quant_Uniform()
{
	unsigned char *rgb = this->To_RGB();

	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;

			img_data[offset_rgba + rr] = img_data[offset_rgba + rr] / 32;
			img_data[offset_rgba + rr] = img_data[offset_rgba + rr] * 32;
			img_data[offset_rgba + gg] = img_data[offset_rgba + gg] / 32;
			img_data[offset_rgba + gg] = img_data[offset_rgba + gg] * 32;
			img_data[offset_rgba + bb] = img_data[offset_rgba + bb] / 64;
			img_data[offset_rgba + bb] = img_data[offset_rgba + bb] * 64;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using populosity quantization.  
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Quant_Populosity()
{
	unsigned char *rgb = this->To_RGB();

	int count[32][32][32];
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			for (int k = 0; k < 32; k++)
			{
				count[i][j][k] = 0;
			}
		}
	}
	int popularC1[256];
	int popularC2[256];
	int popularC3[256];
	//uniform quantization 256bits to 32bits ,and count the most popular 256 colors 
	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;

			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = img_data[offset_rgba + k] / 8;
			
			count[img_data[offset_rgba + 0]]
				[img_data[offset_rgba + 1]]
				[img_data[offset_rgba + 2]]++;
			img_data[offset_rgba + aa] = WHITE;
		}
	}
	for (int i = 0; i < 256; i++)
	{
		int max = 0;
		int c[3];
		for (int j = 0; j < 32; j++)
		{
			for (int k = 0; k < 32; k++)
			{
				for (int l = 0; l < 32; l++)
				{
					if (max < count[j][k][l])
					{
						c[0] = j;
						c[1] = k;
						c[2] = l;
						max = count[j][k][l];
					}
				}
			}
		}
		count[c[0]][c[1]][c[2]] = 0;
		popularC1[i] = c[0];
		popularC2[i] = c[1];
		popularC3[i] = c[2];
	}

	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;

			int min = 999999;
			unsigned char ans[3];
			for (int i = 0; i < 256; i++)
			{
				int c1 = popularC1[i];
				int c2 = popularC2[i];
				int c3 = popularC3[i];
				int point;
				point = sqrt(pow(img_data[offset_rgba + 0] - c1, 2) +
					pow(img_data[offset_rgba + 1] - c2, 2) +
					pow(img_data[offset_rgba + 2] - c3, 2));
				if (min > point)
				{
					min = point;
					ans[0] = c1;
					ans[1] = c2;
					ans[2] = c3;
				}
			}
			for (int i = 0; i < 3; i++)
				img_data[offset_rgba + i] = ans[i]*8;
			img_data[offset_rgba + aa] = WHITE;
		}
	}
	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

//------------------------Dithering------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Dither the image using a threshold of 1/2.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Threshold()
{
	unsigned char *rgb = To_RGB();

	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;
			double gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
			gray /= 256;
			for (int k = 0; k < 3; k++)
			{
				if(gray>0.5)
					img_data[offset_rgba + k] = 255;
				else
					img_data[offset_rgba + k] = 0;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Dither image using random dithering.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Random()
{
	unsigned char *rgb = To_RGB();

	srand((unsigned)time(NULL));
	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;
			double gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
			gray /= 256;
			gray += ((((float)rand()) / (float)RAND_MAX)-0.5)*2/5;
			for (int k = 0; k < 3; k++)
			{
				if (gray>0.5)
					img_data[offset_rgba + k] = 255;
				else
					img_data[offset_rgba + k] = 0;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform Floyd-Steinberg dithering on the image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_FS()
{
	unsigned char *rgb = To_RGB();
	float *newRGB = new float[img_height*img_width * 3];
	for (int i = 0; i < img_height*img_width * 3; i++)
	{
		newRGB[i] = rgb[i];
	}

	bool zigZagFlag = false;
	for (int i = 0; i<img_height; i++)
	{
		for (int j = zigZagFlag ? img_width - 1 : 0; j >= 0 && j < img_width; j += zigZagFlag ? -1 : 1)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;
			double oldPixel = 0.3 * newRGB[offset_rgb + rr] + 0.59 * newRGB[offset_rgb + gg] + 0.11 * newRGB[offset_rgb + bb];
			
			oldPixel /= 256;
			double newPixel;
			if (oldPixel>0.5)
				newPixel = 255;
			else
				newPixel = 0;
			oldPixel *= 256;
			double quant_error = oldPixel - newPixel;
			for (int k = 0; k < 3; k++)
			{
				img_data[offset_rgba + k] = newPixel;
				if (!zigZagFlag)
				{
					if (j + 1 < img_width)
						newRGB[offset_rgb + k+3] += 7.0 / 16 * quant_error;
					if (i + 1 < img_height && j - 1 > 0)
						newRGB[offset_rgb + k - 3 + 3 * img_width] += 3.0 / 16 * quant_error;
					if (i + 1 < img_height)
						newRGB[offset_rgb + k + 3 * img_width] += 5.0 / 16 * quant_error;
					if (i + 1 < img_height && j + 1 < img_width)
						newRGB[offset_rgb + k + 3 * img_width + 3] += 1.0 / 16 * quant_error;
				}
				else
				{
					if(j>0)
						newRGB[offset_rgb + k -3] += 7.0 / 16 * quant_error;
					if (i + 1 < img_height && j - 1 > 0)
						newRGB[offset_rgb + k - 3 + 3 * img_width] += 1.0 / 16 * quant_error;
					if (i + 1 < img_height)
						newRGB[offset_rgb + k + 3 * img_width] += 5.0 / 16 * quant_error;
					if (i + 1 < img_height && j + 1 < img_width)
						newRGB[offset_rgb + k + 3 * img_width + 3] += 3.0 / 16 * quant_error;
				}
				
			}
			img_data[offset_rgba + aa] = WHITE;
		}
		zigZagFlag = !zigZagFlag;
	}

	delete[] rgb;
	delete[] newRGB;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Dither the image while conserving the average brightness.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
int compare(const void * b, const void * a)
{
	return (*(double*)a - *(double*)b);
}
void Application::Dither_Bright()
{
	unsigned char *rgb = To_RGB();
	double sum = 0;
	int count = 0;
	double threshold;
	double *gray=new double[img_width*img_height];
	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			gray[j+i*img_width] = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
			sum += gray[j + i*img_width] /256;
			if (gray[j + i*img_width] == 0)count++;
		}
	}
	sum = sum / (img_height * img_width);
	
	std::qsort(gray, img_height * img_width, sizeof(double), compare);
	threshold = gray[(int)(img_width*img_height*sum)];
	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;
			double cellGray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
			for (int k = 0; k < 3; k++)
			{
				if (cellGray>threshold)
					img_data[offset_rgba + k] = 255;
				else
					img_data[offset_rgba + k] = 0;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}
	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform clustered differing of the image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Cluster()
{
	unsigned char *rgb = To_RGB();
	double mask[4][4] = {
		{ 0.7059, 0.3529, 0.5882, 0.2353 },
		{ 0.0588, 0.9412, 0.8235, 0.4118 },
		{ 0.4706, 0.7647, 0.8824, 0.1176 },
		{ 0.1765, 0.5294, 0.2941, 0.6471 }};

	for (int i = 0; i<img_height; i++)
	{
		for (int j = 0; j<img_width; j++)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;
			double gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
			gray /= 255;
			for (int k = 0; k < 3; k++)
			{
				if (gray>mask[i%4][j%4])
					img_data[offset_rgba + k] = 255;
				else
					img_data[offset_rgba + k] = 0;
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using Floyd-Steinberg dithering over
//  a uniform quantization - the same quantization as in Quant_Uniform.
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Color()
{
	unsigned char *rgb = To_RGB();
	float *newR = new float[img_height*img_width];
	float *newG = new float[img_height*img_width];
	float *newB = new float[img_height*img_width];
	int rTable[] = { 0,36,73,109,146,182,219,255 };
	int gTable[] = { 0,36,73,109,146,182,219,255 };
	int bTable[] = { 0,85,170,255 };
	for (int i = 0; i < img_height*img_width*3; i+=3)
	{
		newR[i/3] = rgb[i+rr];
		newG[i/3] = rgb[i+gg];
		newB[i/3] = rgb[i+bb];
	}

	bool zigZagFlag = false;
	for (int i = 0; i<img_height; i++)
	{
		for (int j = zigZagFlag ? img_width - 1 : 0; j >= 0 && j < img_width; j += zigZagFlag ? -1 : 1)
		{
			int offset_rgb = i*img_width * 3 + j * 3;
			int offset_rgba = i*img_width * 4 + j * 4;
			int index = i*img_width + j;
			double oldPixel = newR[index];
			double newPixel;
			newPixel = rTable[(int)round(oldPixel / (256.0 / 7))];
			double quant_error = oldPixel - newPixel;
			img_data[offset_rgba+rr] = newPixel;
			if (!zigZagFlag)
			{
				if (j + 1 < img_width)
					newR[index+1] += 7.0 / 16 * quant_error;
				if (i + 1 < img_height && j - 1 > 0)
					newR[index-1+img_width] += 3.0 / 16 * quant_error;
				if (i + 1 < img_height)
					newR[index + img_width] += 5.0 / 16 * quant_error;
				if (i + 1 < img_height && j + 1 < img_width)
					newR[index + 1 + img_width] += 1.0 / 16 * quant_error;
			}
			else
			{
				if (j < 0)
					newR[index - 1] += 7.0 / 16 * quant_error;
				if (i + 1 < img_height && j - 1 > 0)
					newR[index - 1 + img_width] += 1.0 / 16 * quant_error;
				if (i + 1 < img_height)
					newR[index + img_width] += 5.0 / 16 * quant_error;
				if (i + 1 < img_height && j + 1 < img_width)
					newR[index + 1 + img_width] += 3.0 / 16 * quant_error;
			}
			
			oldPixel = newG[index];
			newPixel = gTable[(int)round(oldPixel / (256.0 / 7))];
			quant_error = oldPixel - newPixel;
			img_data[offset_rgba+gg] = newPixel;
			if (!zigZagFlag)
			{
				if (j + 1 < img_width)
					newG[index + 1] += 7.0 / 16 * quant_error;
				if (i + 1 < img_height && j - 1 > 0)
					newG[index - 1 + img_width] += 3.0 / 16 * quant_error;
				if (i + 1 < img_height)
					newG[index + img_width] += 5.0 / 16 * quant_error;
				if (i + 1 < img_height && j + 1 < img_width)
					newG[index + 1 + img_width] += 1.0 / 16 * quant_error;
			}
			else
			{
				if (j > 0)
					newG[index - 1] += 7.0 / 16 * quant_error;
				if (i + 1 < img_height && j - 1 > 0)
					newG[index - 1 + img_width] += 1.0 / 16 * quant_error;
				if (i + 1 < img_height)
					newG[index + img_width] += 5.0 / 16 * quant_error;
				if (i + 1 < img_height && j + 1 < img_width)
					newG[index + 1 + img_width] += 3.0 / 16 * quant_error;
			}

			oldPixel = newB[index];
			newPixel = bTable[(int)round(oldPixel / (256.0 / 3))];
			quant_error = oldPixel - newPixel;
			img_data[offset_rgba + bb] = newPixel;
			if (!zigZagFlag)
			{
				if (j + 1 < img_width)
					newB[index + 1] += 7.0 / 16 * quant_error;
				if (i + 1 < img_height && j - 1 > 0)
					newB[index - 1 + img_width] += 3.0 / 16 * quant_error;
				if (i + 1 < img_height)
					newB[index + img_width] += 5.0 / 16 * quant_error;
				if (i + 1 < img_height && j + 1 < img_width)
					newB[index + 1 + img_width] += 1.0 / 16 * quant_error;
			}
			else
			{
				if (j > 0)
					newB[index - 1] += 7.0 / 16 * quant_error;
				if (i + 1 < img_height && j - 1 > 0)
					newB[index - 1 + img_width] += 1.0 / 16 * quant_error;
				if (i + 1 < img_height)
					newB[index + img_width] += 5.0 / 16 * quant_error;
				if (i + 1 < img_height && j + 1 < img_width)
					newB[index + 1 + img_width] += 3.0 / 16 * quant_error;
			}

			img_data[offset_rgba + aa] = WHITE;
		}
		zigZagFlag = !zigZagFlag;
	}

	delete[] rgb;
	delete[] newR;
	delete[] newG;
	delete[] newB;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

//------------------------Filter------------------------

///////////////////////////////////////////////////////////////////////////////
//
//     Filtering the img_data array by the filter from the parameters
//
///////////////////////////////////////////////////////////////////////////////
void Application::filtering( double filter[][5] )
{
	unsigned char *rgb = this->To_RGB();
	double *newrgb = new double[img_width*img_height * 3];
	double sum=0;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			sum += filter[i][j];
		}
	}
	for (int c = 0; c < 3; c++)	//loop for RBG three colors.
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)	//loop for every pixel.
			{
				int offset_rgba = i*img_width * 4 + j * 4;
				int index = (i*img_width + j) * 3 + c;
				newrgb[index] = 0;
				for (int k = -2; k <= 2; k++)
				{
					for (int l = -2; l <= 2; l++)	//loop for 5*5 filter
					{
						if(!((i+k)<0||(i+k)>=img_height||(j+l)<0||(j+l)>=img_width))
							newrgb[index] += rgb[index + (k*img_width + l) * 3]*filter[k+2][l+2] / sum;
					}
				}
				img_data[offset_rgba + c] = newrgb[index];
				img_data[offset_rgba + aa] = WHITE;
			}
		}
	}

	delete[] rgb;
	delete[] newrgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

void Application::filtering( double **filter, int n )
{
	unsigned char *rgb = this->To_RGB();
	double *newrgb = new double[img_width*img_height * 3];
	double sum = 0;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			sum += filter[i][j];
		}
	}
	for (int c = 0; c < 3; c++)	//loop for RBG three colors.
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)	//loop for every pixel.
			{
				int offset_rgba = i*img_width * 4 + j * 4;
				int index = (i*img_width + j) * 3 + c;
				newrgb[index] = 0;
				for (int k = ((n-1)/2)*(-1); k <= ((n - 1) / 2); k++)
				{
					for (int l = ((n - 1) / 2)*(-1); l <= ((n - 1) / 2); l++)	//loop for 5*5 filter
					{
						if (!((i + k)<0 || (i + k) >= img_height || (j + l)<0 || (j + l) >= img_width))
							newrgb[index] += rgb[index + (k*img_width + l) * 3] * filter[k + ((n - 1) / 2)][l + ((n - 1) / 2)] / sum;
					}
				}
				img_data[offset_rgba + c] = newrgb[index];
				img_data[offset_rgba + aa] = WHITE;
			}
		}
	}

	delete[] rgb;
	delete[] newrgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 box filter on this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Box()
{
	double f[5][5] = {
		{ 0.04, 0.04, 0.04, 0.04, 0.04 },
		{ 0.04, 0.04, 0.04, 0.04, 0.04 },
		{ 0.04, 0.04, 0.04, 0.04, 0.04 },
		{ 0.04, 0.04, 0.04, 0.04, 0.04 },
		{ 0.04, 0.04, 0.04, 0.04, 0.04 },
	};
	Application::filtering(f);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 Bartlett filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Bartlett()
{
	double f[5][5] = {
		{ 1, 2, 3, 2, 1 },
		{ 2, 4, 6, 4, 2 },
		{ 3, 6, 9, 6, 3 },
		{ 2, 4, 6, 4, 2 },
		{ 1, 2, 3, 2, 1 },
	};
	Application::filtering(f);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Gaussian()
{
	double f[5][5] = {
		{ 1, 4 , 6 , 4 , 1 },
		{ 4, 16, 24, 16, 4 },
		{ 6, 24, 36, 24, 6 },
		{ 4, 16, 24, 16, 4 },
		{ 1, 4 , 6 , 4 , 1 },
	};
	Application::filtering(f);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform NxN Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Gaussian_N( unsigned int N )
{
	double **f = new double*[N];
	//generate first row (column)
	double **buf = new double*[N];
	for (int i = 0; i < N; i++)
	{
		f[i] = new double[N];
		buf[i] = new double[N];
		f[i][0] = 0;
		buf[i][0] = 0;
	}
	f[0][0] = 1;
	buf[0][0] = 1;
	for (int i = 1; i < N; i++)
	{
		for (int j = 1; j <= i; j++)
		{
			buf[j][0] = f[j - 1][0] + f[j][0];
		}
		for (int j = 0; j < N; j++)
		{
			f[j][0] = buf[j][0];
		}
	}

	//put first row into first column.
	for (int i = 0; i < N; i++)
	{
		f[i][0] = f[0][i];
	}

	//fill all box.
	for (int i = 1; i < N; i++)
	{
		for (int j = 1; j < N; j++)
		{
			f[i][j] = f[i][0] * f[0][j];
		}
	}
	Application::filtering(f, N);
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 edge detect (high pass) filter on this image.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Edge()
{
	double filter[5][5] = {
		{ 1, 4 , 6 , 4 , 1 },
		{ 4, 16, 24, 16, 4 },
		{ 6, 24, 36, 24, 6 },
		{ 4, 16, 24, 16, 4 },
		{ 1, 4 , 6 , 4 , 1 },
	};
	unsigned char *rgb = this->To_RGB();
	double *newrgb = new double[img_width*img_height * 3];
	double sum = 256;
	for (int k = 0; k < 3; k++)	//loop for RGB three colors.
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)	//loop for every pixel.
			{
				int offset_rgba = i*img_width * 4 + j * 4;
				int index = (i*img_width + j) * 3 + k;
				newrgb[index] = 0;
				for (int m = -2; m <= 2; m++)
				{
					for (int l = -2; l <= 2; l++)	//loop for 5*5 filter
					{
						if (!((i + m)<0 || (i + m) >= img_height || (j + l)<0 || (j + l) >= img_width))
							newrgb[index] += (double)(rgb[index + (m*img_width + l) * 3]) * filter[m + 2][l + 2] / sum;
					}
				}
			}
		}
	}
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgba = i*img_width * 4 + j * 4;
			for (int k = 0; k < 3; k++)
			{
				int index = (i*img_width + j) * 3 + k;
				if (img_data[offset_rgba + k] - newrgb[index] < 0)
					img_data[offset_rgba + k] = 0;
				else if((img_data[offset_rgba + k] - newrgb[index] >255))
					img_data[offset_rgba + k] = 255;
				else
					img_data[offset_rgba + k] -= newrgb[index];
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	delete[] newrgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform a 5x5 enhancement filter to this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Enhance()
{
	double filter[5][5] = {
		{ 1, 4 , 6 , 4 , 1 },
		{ 4, 16, 24, 16, 4 },
		{ 6, 24, 36, 24, 6 },
		{ 4, 16, 24, 16, 4 },
		{ 1, 4 , 6 , 4 , 1 },
	};
	unsigned char *rgb = this->To_RGB();
	double *newrgb = new double[img_width*img_height * 3];
	double sum = 256;
	for (int k = 0; k < 3; k++)	//loop for RGB three colors.
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)	//loop for every pixel.
			{
				int offset_rgba = i*img_width * 4 + j * 4;
				int index = (i*img_width + j) * 3 + k;
				newrgb[index] = 0;
				for (int m = -2; m <= 2; m++)
				{
					for (int l = -2; l <= 2; l++)	//loop for 5*5 filter
					{
						if (!((i + m)<0 || (i + m) >= img_height || (j + l)<0 || (j + l) >= img_width))
							newrgb[index] += (double)(rgb[index + (m*img_width + l) * 3]) * filter[m + 2][l + 2] / sum;
					}
				}
			}
		}
	}
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgba = i*img_width * 4 + j * 4;
			for (int k = 0; k < 3; k++)
			{
				int index = (i*img_width + j) * 3 + k;
				if (img_data[offset_rgba + k]*2 - newrgb[index] < 0)
					img_data[offset_rgba + k] = 0;
				else if ((img_data[offset_rgba + k]*2 - newrgb[index] >255))
					img_data[offset_rgba + k] = 255;
				else
					img_data[offset_rgba + k] = img_data[offset_rgba + k]*2 - newrgb[index];
			}
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	delete[] newrgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

//------------------------Size------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Halve the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Half_Size()
{
	double f[3][3] = {
		{1 / 16.0 ,1 / 8.0, 1 / 16.0},
		{1 / 8.0  ,1 / 4.0 ,1 / 8.0 },
		{1 / 16.0 ,1 / 8.0, 1 / 16.0}
	};
	int new_width = img_width / 2, new_height = img_height / 2;
	unsigned char *new_img = new unsigned char[new_width*new_height*4];

	for (int i = 0; i < new_height; i++)
	{
		for (int j = 0; j < new_width; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				int index = (i*new_width + j) * 4 + k;
				double sum = 0;
				new_img[index] = 0;

				new_img[index - k + aa] = WHITE;
				for (int l = -1; l <= 1; l++)
				{
					for (int m = -1; m <= 1; m++)
					{
						int orgIndex = ((i * 2 + l)*img_width + (j * 2 + m)) * 4 + k;
						if(!((i * 2 + l)<0|| (i * 2 + l)>=img_height|| (j * 2 + m)<0|| (j * 2 + m)>=img_width))
							sum += f[l + 1][m + 1] * img_data[orgIndex];
					}
				}
				new_img[index] = sum;
			}
		}
	}
	img_data = new_img;
	img_height = new_height;
	img_width = new_width;
	ui_instance->ui.label->setFixedHeight(new_height);
	ui_instance->ui.label->setFixedWidth(new_width);
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Double the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Double_Size()
{
	double f1[3][3] = {
		{ 1 / 16.0 ,1 / 8.0, 1 / 16.0 },
		{ 1 / 8.0  ,1 / 4.0 ,1 / 8.0 },
		{ 1 / 16.0 ,1 / 8.0, 1 / 16.0 }
	};
	double f2[4][4] = {
		{ 1 / 64.0 ,3 / 64.0, 3 / 64.0 ,1 / 64.0 },
		{ 3 / 64.0 ,9 / 64.0, 9 / 64.0 ,3 / 64.0 },
		{ 3 / 64.0 ,9 / 64.0, 9 / 64.0 ,3 / 64.0 },
		{ 1 / 64.0 ,3 / 64.0, 3 / 64.0 ,1 / 64.0 },
	};
	double f3[4][3] = {
		{ 1 / 32.0 ,2 / 32.0, 1 / 32.0 },
		{ 3 / 32.0 ,6 / 32.0, 3 / 32.0 },
		{ 3 / 32.0 ,6 / 32.0, 3 / 32.0 },
		{ 1 / 32.0 ,2 / 32.0, 1 / 32.0 },
	};
	double f4[3][4] = {
		{ 1 / 32.0 ,3 / 32.0, 3 / 32.0 ,1 / 32.0 },
		{ 2 / 32.0 ,6 / 32.0, 6 / 32.0 ,2 / 32.0 },
		{ 1 / 32.0 ,3 / 32.0, 3 / 32.0 ,1 / 32.0 },
	};
	int new_width = img_width * 2, new_height = img_height * 2;
	unsigned char *new_img = new unsigned char[new_width*new_height * 4];
	for (int i = 0; i < new_height; i++)
	{
		for (int j = 0; j < new_width; j++)
		{
			for (int c = 0; c < 3; c++)
			{
				int index = (i*new_width + j) * 4 + c;
				double sum = 0;
				new_img[index] = 0;
				new_img[index - c + aa] = WHITE;
				if (i % 2 == 0 && j % 2 == 0)
				{
					for (int l = -1; l <= 1; l++)
					{
						for (int m = -1; m <= 1; m++)
						{
							int orgIndex = ((i/2 + l)*img_width + (j/2 + m)) * 4 + c;
							if (orgIndex >= 0 && orgIndex < (img_height*img_width * 4))
								sum += img_data[orgIndex] * f1[l + 1][m + 1];
						}
					}
					new_img[index] = sum;
				}
				else if (i % 2 == 1 && j % 2 == 1)
				{
					for (int l = i / 2 - 1; l <= i / 2 + 2; l++)
					{
						for (int m = j / 2 - 1; m <= j / 2 + 2; m++)
						{
							int orgIndex = (l*img_width + m) * 4 + c;
							if (orgIndex >= 0 && orgIndex < (img_height*img_width * 4))
								sum += img_data[orgIndex] * f2[l -( i / 2 - 1)][m- (j / 2 - 1)];
						}
					}
					new_img[index] = sum;
				}
				else if (i % 2 == 0 && j % 2 == 1)
				{
					for (int l = i / 2 - 1; l <= i / 2 + 1; l++)
					{
						for (int m = j / 2 - 1; m <= j / 2 + 2; m++)
						{
							int orgIndex = (l*img_width + m) * 4 + c;
							if (orgIndex >= 0 && orgIndex < (img_height*img_width * 4))
								sum += img_data[orgIndex] * f4[l-(i / 2 - 1)][m-(j / 2 - 1)];
						}
					}
					new_img[index] = sum;
				}
				else
				{
					for (int l = i / 2 - 1; l <= i / 2 + 2; l++)
					{
						for (int m = j / 2 - 1; m <= j / 2 + 1; m++)
						{
							int orgIndex = (l*img_width + m) * 4 + c;
							if (orgIndex >= 0 && orgIndex < (img_height*img_width * 4))
								sum += img_data[orgIndex] * f3[l - (i / 2 - 1)][m - (j / 2 - 1)];
						}
					}
					new_img[index] = sum;
				}
			}
		}
	}

	img_data = new_img;
	img_height = new_height;
	img_width = new_width;
	ui_instance->ui.label->setFixedHeight(new_height);
	ui_instance->ui.label->setFixedWidth(new_width);
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  resample_src for resize and rotate
//
///////////////////////////////////////////////////////////////////////////////
void Application::resample_src(int u, int v, float ww, unsigned char* rgba)
{
	
}

///////////////////////////////////////////////////////////////////////////////
//
//  Scale the image dimensions by the given factor.  The given factor is 
//	assumed to be greater than one.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Resize( float scale )
{
	int new_height = img_height*scale;
	int new_width = img_width*scale;
	unsigned char *new_image = new unsigned char[new_height*new_width * 4];

	for (int i = 0; i < new_height; i++)
	{
		for (int j = 0; j < new_width; j++)
		{
			int orih, oriw;
			orih = i / scale;
			oriw = j / scale;
			for (int k = 0; k < 4; k++)
			{
				new_image[(i*new_width + j) * 4 + k] = img_data[(orih*img_width + oriw) * 4 + k];
			}
		}
	}
	img_data = new_image;
	img_height = new_height;
	img_width = new_width;
	ui_instance->ui.label->setFixedHeight(new_height);
	ui_instance->ui.label->setFixedWidth(new_width);
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rotate the image clockwise by the given angle.  Do not resize the 
//  image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Rotate( float angleDegrees )
{
	unsigned char *new_image = new unsigned char[img_width*img_height * 4];

	angleDegrees = angleDegrees / 180 * acos(-1);

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int movex = j - img_width / 2;
			int movey = i - img_height / 2;
			int orix = cos(-angleDegrees)*movex - sin(-angleDegrees)*movey;
			orix += img_width / 2;
			int oriy = sin(-angleDegrees)*movex + cos(-angleDegrees)*movey;
			oriy += img_height / 2;
			if(orix>=0&&orix<img_width&&oriy>=0&&oriy<img_height)
				for (int k = 0; k < 4; k++)
				{
					new_image[(i*img_width + j) * 4 + k] = img_data[(oriy*img_width + orix) * 4 + k];
				}
		}
	}
	img_data = new_image;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

//------------------------Composing------------------------


void Application::loadSecondaryImge( QString filePath )
{
	mImageSrcSecond.load(filePath);

	renew();

	img_data2 = mImageSrcSecond.bits();
	img_width2 = mImageSrcSecond.width();
	img_height2 = mImageSrcSecond.height();
}

//////////////////////////////////////////////////////////////////////////
//
//	Composite the image A and image B by Over, In, Out, Xor and Atom. 
//
//////////////////////////////////////////////////////////////////////////
void Application::Comp_image( int tMethod )
{
	if (tMethod == 0)
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					double a = (double)img_data[(i*img_width + j)*4 + aa] / 255;
					double temp= (double)img_data[(i*img_width + j)*4 + k] * a + (double)img_data2[(i*img_width + j)*4 + k] * (1 - a);
					img_data[(i*img_width + j)*4 + k] = temp;
				}
				img_data[(i*img_width + j) * 4 + aa] = 255;
			}
		}
	}
	else if (tMethod == 1)
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					double a = (double)img_data2[(i*img_width + j) * 4 + aa] / 255;
					double temp = (double)img_data[(i*img_width + j) * 4 + k] * a;
					img_data[(i*img_width + j) * 4 + k] = temp;
				}
				img_data[(i*img_width + j) * 4 + aa] = 255;
			}
		}
	}
	else if (tMethod == 2)
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					double a = (double)img_data2[(i*img_width + j) * 4 + aa] / 255;
					a = 1 - a;
					double temp = (double)img_data[(i*img_width + j) * 4 + k] * a;
					img_data[(i*img_width + j) * 4 + k] = temp;
				}
				img_data[(i*img_width + j) * 4 + aa] = 255;
			}
		}
	}
	else if (tMethod == 3)
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					double a = (double)img_data2[(i*img_width + j) * 4 + aa] / 255;
					double temp = (double)img_data[(i*img_width + j) * 4 + k] * a + (double)img_data2[(i*img_width + j) * 4 + k] * (1 - a);
					img_data[(i*img_width + j) * 4 + k] = temp;
				}
				img_data[(i*img_width + j) * 4 + aa] = 255;
			}
		}
	}
	else if (tMethod == 4)
	{
		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					double a = (double)img_data[(i*img_width + j) * 4 + aa] / 255;
					double b = (double)img_data2[(i*img_width + j) * 4 + aa] / 255;
					double temp = (double)img_data[(i*img_width + j) * 4 + k] * (1 - b) + (double)img_data2[(i*img_width + j) * 4 + k] * (1 - a);
					img_data[(i*img_width + j) * 4 + k] = temp;
				}
				img_data[(i*img_width + j) * 4 + aa] = 255;
			}
		}
	}
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite the current image over the given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Over()
{
	if (img_height == img_height2 && img_width == img_width2)
	{
		Comp_image(0);
	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "in" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_In()
{
	if (img_height == img_height2 && img_width == img_width2)
	{
		Comp_image(1);
	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "out" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Out()
{
	if (img_height == img_height2 && img_width == img_width2)
	{
		Comp_image(2);
	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite current image "atop" given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Atop()
{
	if (img_height == img_height2 && img_width == img_width2)
	{
		Comp_image(3);
	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image with given image using exclusive or (XOR).  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Xor()
{
	if (img_height == img_height2 && img_width == img_width2)
	{
		Comp_image(4);
	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

//------------------------NPR------------------------

///////////////////////////////////////////////////////////////////////////////
//
//      Run simplified version of Hertzmann's painterly image filter.
//      You probably will want to use the Draw_Stroke funciton and the
//      Stroke class to help.
// Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::NPR_Paint()
{
	const int NUM_SIZE = 3;
	int R[NUM_SIZE] = { 7,3,1 };	//radius of brush
	unsigned char *referenceImage = new unsigned char[img_width*img_height * 3]; //image has been handled.
	unsigned char *sourceImage = new unsigned char[img_width*img_height * 3]; //new original image.

	//Save the image.
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < img_height; j++)
			for (int k = 0; k < img_width; k++)
				sourceImage[i + (j*img_width + k) * 3] = img_data[i + (j*img_width + k) * 4];

	//Paint canvas.
	for (int i = 0; i < NUM_SIZE; i++)
	{
		Filter_Gaussian_N(R[i]);
		for (int l = 0; l < 3; l++)
			for (int j = 0; j < img_height; j++)
				for (int k = 0; k < img_width; k++)
				{
					referenceImage[l + (j*img_width + k) * 3] = img_data[l + (j*img_width + k) * 4];
					img_data[l + (j*img_width + k) * 4] = sourceImage[l + (j*img_width + k) * 3];
				}
		NPR_Paint_Layer(img_data, referenceImage, R[i]);
	}
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
void Application::NPR_Paint_Layer( unsigned char *tCanvas, unsigned char *tReferenceImage, int tBrushSize )
{
	Stroke s;
	std::vector<Stroke> S;
	//Compute differnet for each piexl.
	unsigned char *d = new unsigned char[img_width*img_height * 3];
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < img_height; j++)
			for (int k = 0; k < img_width; k++)
				d[i + (j*img_width + k) * 3] = abs(tCanvas[i + (j*img_width + k) * 4] - tReferenceImage[i + (j*img_width + k) * 3]);
	
	//Compute max different pixel for each grid.
	//And create stroke.
	for (int i = 0; i < img_height-tBrushSize; i+= tBrushSize)
	{
		for (int j = 0; j < img_width- tBrushSize; j+=tBrushSize)
		{
			int x1, y1;
			double max_error = -1;
			for (int h = i; h < i+tBrushSize; h++)
			{
				for (int w = j; w < j+tBrushSize; w++)
				{
					double temp = 0;
					for (int k = 0; k < 3; k++)
					{
						temp += d[k + (h*img_width + w) * 3];
					}
					if (max_error < temp)
					{
						max_error = temp;
						x1 = w;
						y1 = h;
					}
				}
			}
			
			s = Stroke(tBrushSize, x1+ (rand()%3)-1, y1 + (rand() % 3) - 1, tReferenceImage[(y1*img_width + x1) * 3 + rr], tReferenceImage[(y1*img_width + x1) * 3 + gg], tReferenceImage[(y1*img_width + x1) * 3 + bb], WHITE);
			S.push_back(s);
		}
	}
	std::random_shuffle(S.begin(), S.end());
	for (int i = 0; i < S.size(); i++)
	{
		Paint_Stroke(S[i]);
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//      Helper function for the painterly filter; paint a stroke at
// the given location
//
///////////////////////////////////////////////////////////////////////////////
void Application::Paint_Stroke( const Stroke& s )
{
	int radius_squared = (int)s.radius * (int)s.radius;
	for (int x_off = -((int)s.radius); x_off <= (int)s.radius; x_off++) 
	{
		for (int y_off = -((int)s.radius); y_off <= (int)s.radius; y_off++) 
		{
			int x_loc = (int)s.x + x_off;
			int y_loc = (int)s.y + y_off;

			// are we inside the circle, and inside the image?
			if ((x_loc >= 0 && x_loc < img_width && y_loc >= 0 && y_loc < img_height)) 
			{
				int dist_squared = x_off * x_off + y_off * y_off;
				int offset_rgba = (y_loc * img_width + x_loc) * 4;

				if (dist_squared <= radius_squared) 
				{
					img_data[offset_rgba + rr] = s.r;
					img_data[offset_rgba + gg] = s.g;
					img_data[offset_rgba + bb] = s.b;
					img_data[offset_rgba + aa] = s.a;
				} 
				else if (dist_squared == radius_squared + 1) 
				{
					img_data[offset_rgba + rr] = (img_data[offset_rgba + rr] + s.r) / 2;
					img_data[offset_rgba + gg] = (img_data[offset_rgba + gg] + s.g) / 2;
					img_data[offset_rgba + bb] = (img_data[offset_rgba + bb] + s.b) / 2;
					img_data[offset_rgba + aa] = (img_data[offset_rgba + aa] + s.a) / 2;
				}
			}
		}
	}
}





///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke() {}

///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke(unsigned int iradius, unsigned int ix, unsigned int iy,
	unsigned char ir, unsigned char ig, unsigned char ib, unsigned char ia) :
radius(iradius),x(ix),y(iy),r(ir),g(ig),b(ib),a(ia)
{
}



