#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
	
int read_jpeg_file(char *filename, unsigned char **raw_image_out, struct jpeg_decompress_struct *cinfo)
{
	// open the file
	FILE *infile = fopen(filename, "rb");
	if (!infile)
	{
		fprintf(stderr, "Error opening jpeg file %s\n", filename );
		return -1;
	}
	
	// set up the standard libjpeg error handler
	struct jpeg_error_mgr jerr;
	cinfo->err=jpeg_std_error(&jerr);
	
	// start decompression
	jpeg_create_decompress(cinfo);
	jpeg_stdio_src(cinfo, infile);
	jpeg_read_header(cinfo, TRUE);
	jpeg_start_decompress(cinfo);

	// allocate memory to hold the uncompressed image
	unsigned char *raw_image=(unsigned char*)malloc(cinfo->output_width*cinfo->output_height*cinfo->num_components);
	
	// now actually read the jpeg into the raw buffer
	JSAMPROW row_pointer=(unsigned char *)malloc(cinfo->output_width*cinfo->num_components);
	
	// read one scan line at a time
	unsigned long location=0;	
	while(cinfo->output_scanline < cinfo->image_height)
	{
		jpeg_read_scanlines(cinfo, &row_pointer, 1);
		for(int i=0; i<cinfo->image_width*cinfo->num_components;i++) 
			raw_image[location++]=row_pointer[i];
	}
	
	// wrap up decompression, destroy objects, free pointers and close open files
	jpeg_finish_decompress(cinfo);
	jpeg_destroy_decompress(cinfo);
	free(row_pointer);
	fclose(infile);
	
	// finish up (caller must dealloc if necessary)
	*raw_image_out = raw_image;
	return 1;
}

int main(int argc, char *argv[])
{
	char *file1=NULL;
	char *file2=NULL;
	
	if (argc>2) {
		file1=argv[1];
		file2=argv[2];		
	}	
	else {
		fprintf(stderr, "Usage: analyse current.jpg previous.jpg\nOutput is '[brightness of current] [difference between current and previous]'");
	}

	unsigned char *raw_image_1 = NULL;
	unsigned char *raw_image_2 = NULL;
	struct jpeg_decompress_struct cinfo_1;
	struct jpeg_decompress_struct cinfo_2;
	
	if( read_jpeg_file( file1, &raw_image_1, &cinfo_1 ) > 0 && read_jpeg_file( file2, &raw_image_2, &cinfo_2 ) > 0  ) 
	{
		if (cinfo_1.image_width==cinfo_2.image_width &&
			cinfo_1.image_height==cinfo_2.image_height &&
			cinfo_2.num_components==cinfo_2.num_components)
		{		
				const unsigned int width=cinfo_1.image_width;
				const unsigned int height=cinfo_1.image_height;
				const unsigned int comps=cinfo_1.num_components;			
									
				double sum=0;
				double diffsum=0;				
				double points=0;
				
				// only measure every 10th pixel
				for (size_t row=5; row<height; row+=10) {			
					for (size_t col=5; col<width; col+=10) {
						// count points
						points += 1;			
						
						// get pixel values
						unsigned char *p1=raw_image_1+row*width*comps+col*comps;
						double r1=p1[0];
						double g1=p1[1];
			 			double b1=p1[2];																		
						unsigned char *p2=raw_image_2+row*width*comps+col*comps;												
						double r2=p2[0];
						double g2=p2[1];
			 			double b2=p2[2];						
						
						// brightness
			 			sum += (r1/255.0 + g1/255.0 + b1/255.0) / 3.0;
						
						// difference
						double dr=(r1-r2)/255.0;
						double dg=(g1-g2)/255.0;
						double db=(b1-b2)/255.0;	
						double pixel_diff=(dr * dr + dg * dg + db * db)/3.0;						
						if (pixel_diff>0.075) {
							diffsum+=pixel_diff;
						}						
					}
				}
				
				unsigned int b=(unsigned int)((sum/points)*1000000);
				unsigned int d=(unsigned int)((diffsum/points)*1000000);				
				printf("%d %d", b, d);										
		}
	}
	else return -1;
	return 0;
}


