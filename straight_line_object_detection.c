

int
main(void)
{

	msc_inti();

	//
	//	Read a BMP file from the mass storage device
	//
	int i,j;
	int maxR;

	mem_init();


	// Read Image from USB as a 2-d matrix and store it in "image"
	bitmap = usb_imread("image5.bmp");
	image = m_malloc(InfoHeader.Height*InfoHeader.Width*sizeof(unsigned char));
	maxR= sqrt(InfoHeader.Height*InfoHeader.Height+InfoHeader.Width*InfoHeader.Width);



	for(i = 0; i< InfoHeader.Height; i++)
		for(j = 0;j < InfoHeader.Width; j++)
			if(bitmap[(i*InfoHeader.Width+j)*3] < 10)
				image[i*InfoHeader.Width+j] = 1;
			else
				image[i*InfoHeader.Width+j] = 0;
	m_free(bitmap);
	//	 Print the imported image
	votes = m_malloc(13*(maxR+100)*sizeof(unsigned char));
	for(i=0;i<13*(maxR+100);i++)
	{
		votes[i]=0;
	}
	for(i = 0; i< InfoHeader.Height; i++){
		for(j = 0;j < InfoHeader.Width; j++){
			if(image[i*InfoHeader.Width+j]==1)
			{
				for(k=0;k<13;k++)
				{
					int r=i*sin(k*15.0*(3.1415926/180.0))+j*cos(k*15.0*(3.1415926/180.0))+100;

					votes[r*13+k]=votes[r*13+k]+1;
				}
			}
		}
	}

	/*for(i=0;i<13*(maxR+100)*sizeof(unsigned char);i++)
	{
		if(i%13==0)
			printf("\n");
		printf("%d",votes[i]);
	}*/

	//Free memory

	m_free(image);

}

