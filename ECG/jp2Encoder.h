#include <jasper/jasper.h>
#include <jasper/jas_image.h>

class jp2Encoder{
private:
	jas_stream_t* pStream;  //the pointer to the allocated stream, which stores the encoded data
	jas_image_t* pImage; //the image data structure which stores the raw ECG image
	jas_matrix_t* ref_data;
	int jp2Size;  //the size of the encoded data stored in the stream
	unsigned char* p_jp2Data;

	int width, height;
public:
	jp2Encoder(int, int);
	~jp2Encoder();
	
	int Encode();
	int writeCompon(int*, int, int);
	int getjp2Size();
	unsigned char* getjp2Data();
};
