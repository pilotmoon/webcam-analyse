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
	
	// set up the libjpeg error handler
	struct jpeg_error_mgr jerr;
	cinfo->err=jpeg_std_error(&jerr);
	
	// start decompression
	jpeg_create_decompress(cinfo);
	jpeg_stdio_src(cinfo, infile);
	jpeg_read_header(cinfo, TRUE);
	jpeg_start_decompress(cinfo);

	// allocate memory to hold the uncompressed image (caller must dealloc if necessary)
	*raw_image_out=(unsigned char *)malloc(cinfo->output_width*cinfo->output_height*cinfo->num_components);
	unsigned char *row=*raw_image_out;
	
	// read one scan line at a time
	while(cinfo->output_scanline<cinfo->image_height)
	{
		jpeg_read_scanlines(cinfo, &row, 1);
		row+=cinfo->output_width*cinfo->num_components;
	}
	
	// wrap up
	jpeg_finish_decompress(cinfo);
	jpeg_destroy_decompress(cinfo);
	fclose(infile);
	
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
		fprintf(stderr, "Usage: analyse current.jpg previous.jpg\n"
			"Output format is: brightness difference\n");
	}

	unsigned char *raw_image_1=NULL;
	unsigned char *raw_image_2=NULL;
	struct jpeg_decompress_struct cinfo_1;
	struct jpeg_decompress_struct cinfo_2;
	
	if(read_jpeg_file(file1, &raw_image_1, &cinfo_1)>0&&
	   read_jpeg_file(file2, &raw_image_2, &cinfo_2)>0) 
	{
		if(cinfo_1.image_width==cinfo_2.image_width&&
		   cinfo_1.image_height==cinfo_2.image_height&&
		   cinfo_1.num_components==cinfo_2.num_components)
		{										
			double sum=0;
			double diffsum=0;				
			double points=0;
			
			// only measure every 10th pixel
			for (size_t row=5; row<cinfo_1.image_height; row+=10) {			
				for (size_t col=5; col<cinfo_1.image_width; col+=10) {
					// count points
					points += 1;			
					
					// get pixel values of current image
					unsigned char *p1=raw_image_1+row*cinfo_1.image_width*cinfo_1.num_components+col*cinfo_1.num_components;
					double r1=p1[0];
					double g1=p1[1];
		 			double b1=p1[2];																		

					// brightness
		 			sum += (r1/255.0 + g1/255.0 + b1/255.0) / 3.0;
					
					// get pixel values of previous image						
					unsigned char *p2=raw_image_2+row*cinfo_1.image_width*cinfo_1.num_components+col*cinfo_1.num_components;												
					double r2=p2[0];
					double g2=p2[1];
		 			double b2=p2[2];						
					
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
			
			unsigned int brightness=(unsigned int)((sum/points)*1000000);
			unsigned int difference=(unsigned int)((diffsum/points)*1000000);				
			printf("%d %d", brightness, difference);										
		}
		return 0;
	}
	else {
		return -1;
	}
}


