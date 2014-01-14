#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
	
int read_jpeg_file(har *filename, unsigned char **raw_image_out, struct jpeg_decompress_struct *cinfo)
{
	struct jpeg_error_mgr jerr;
	
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];
	
	FILE *infile = fopen( filename, "rb" );
	unsigned long location = 0;
	int i = 0;
	
	if ( !infile )
	{
		fprintf(stderr, "Error opening jpeg file %s\n", filename );
		return -1;
	}
	/* here we set up the standard libjpeg error handler */
	cinfo->err = jpeg_std_error( &jerr );
	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress( cinfo );
	/* this makes the library read from infile */
	jpeg_stdio_src( cinfo, infile );
	/* reading the image header which contains image information */
	jpeg_read_header( cinfo, TRUE );
	/* Uncomment the following to output image information, if needed. */
	
	// printf( "JPEG File Information: \n" );
	// printf( "Image width and height: %d pixels and %d pixels.\n", cinfo->image_width, cinfo->image_height );
	// printf( "Color components per pixel: %d.\n", cinfo->num_components );
	// printf( "Color space: %d.\n", cinfo->jpeg_color_space );
	
	/* Start decompression jpeg here */
	jpeg_start_decompress( cinfo );

	/* allocate memory to hold the uncompressed image */
	unsigned char *raw_image = (unsigned char*)malloc( cinfo->output_width*cinfo->output_height*cinfo->num_components );
	/* now actually read the jpeg into the raw buffer */
	row_pointer[0] = (unsigned char *)malloc( cinfo->output_width*cinfo->num_components );
	/* read one scan line at a time */
	while( cinfo->output_scanline < cinfo->image_height )
	{
		jpeg_read_scanlines( cinfo, row_pointer, 1 );
		for( i=0; i<cinfo->image_width*cinfo->num_components;i++) 
			raw_image[location++] = row_pointer[0][i];
	}
	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress( cinfo );
	jpeg_destroy_decompress( cinfo );
	free( row_pointer[0] );
	fclose( infile );
	/* yup, we succeeded! */
	
	*raw_image_out = raw_image;
	return 1;
}

int main(int argc, char *argv[])
{
	char *file1 = "example1.jpg";
	char *file2 = "example2.jpg";	
	if (argc>2) {
		file1=argv[1];
		file2=argv[2];		
	}
	
	// file1 is current, file2 is previous
	//printf("filenames: %s, $s\n", file1, file2);

	unsigned char *raw_image_1 = NULL;
	unsigned char *raw_image_2 = NULL;
	struct jpeg_decompress_struct cinfo_1;
	struct jpeg_decompress_struct cinfo_2;
	
	/* Try opening a jpeg*/
	if( read_jpeg_file( file1, &raw_image_1, &cinfo_1 ) > 0 && read_jpeg_file( file2, &raw_image_2, &cinfo_2 ) > 0  ) 
	{
		if (cinfo_1.image_width==cinfo_2.image_width &&
			cinfo_1.image_height==cinfo_2.image_height &&
			cinfo_2.num_components==cinfo_2.num_components) {		
				unsigned int width=cinfo_1.image_width;
				unsigned int height=cinfo_1.image_height;
				unsigned int comps=cinfo_1.num_components;								
				double sum=0;
				double diffsum=0;				
				double points=0;
				for (size_t row=5; row<height; row+=10) {			
					for (size_t col=5; col<width; col+=10) {
						points += 1;			
						
						unsigned char *p1=raw_image_1+row*width*comps+col*comps;
						double r1=*(p1++);
						double g1=*(p1++);
			 			double b1=*p1;						
						
						unsigned char *p2=raw_image_2+row*width*comps+col*comps;												
						double r2=*(p2++);
						double g2=*(p2++);
			 			double b2=*p2;						
						
			 			sum += (r1/255.0 + g1/255.0 + b1/255.0) / 3.0;
						
						double dr=(r1-r2)/255.0;
						double dg=(g1-g2)/255.0;
						double db=(b1-b2)/255.0;	
						double pixel_diff=(dr * dr + dg * dg + db * db)/3.0;						
						if (pixel_diff>0.075) {
							diffsum+=pixel_diff;
						}
						
					}
				}
				//printf("Points %f\n", points);	
				unsigned int b=(unsigned int)((sum/points)*1000000);
				unsigned int d=(unsigned int)((diffsum/points)*1000000);				
				//printf("Brightness: %d\n", b);
				//printf("Difference: %d\n", d);
				printf("{\"brightness\": %d, \"difference\": %d}", b, d);										
		}
	}
	else return -1;
	return 0;
}


