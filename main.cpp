/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Ronald Durham

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <thread>
#include <chrono>
#include <iomanip> 
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < 4; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}
/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2);
    int start = get_int(stream, 10);
    int width = get_int(stream, 18);
    int height = get_int(stream, 22);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * 3;
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid 24-bit true color image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // Advance the position to the next pixel
            pos = pos + 3;
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

//
// YOUR FUNCTION DEFINITIONS HERE
//


//-------------HELPER FUNCTIONS------------------------------------------------------------------------


void welcome_screen()
{
    cout << endl;
    cout << "***********************" << endl;
    cout << "     Final Project" << endl;
    cout << "       CSPB 1300" << endl;
    cout << "     Ronald Durham" << endl;
    cout << "***********************" << endl;
    cout << "Welcome to this image" << endl;
    cout << "processing program!" << endl;
}




string image_name()
{
    string filename;
    cout << endl;
    cout << "Please enter a BMP" << endl;
    cout << "filename (sample.bmp): " << endl;
    cin >> filename;
    cout << endl;
    return filename;
}




int image_menu(string filename)
{
    cout << endl;
    cout << "******** MENU *********" << endl;
    cout << "Current Image: (" << filename << ")" << endl;
    cout << "***********************" << endl;
    cout << "1) Vignette" << endl;
    cout << "2) Clarendon" << endl;
    cout << "3) Grayscale" << endl;
    cout << "4) Rotate 90" << endl;
    cout << "5) Rotate 90, x times" << endl;
    cout << "6) Enlarge" << endl;
    cout << "7) High contrast" << endl;
    cout << "8) Lighten" << endl;
    cout << "9) Darken" << endl;
    cout << "10) Black, white, red, green, blue" << endl;
    cout << "11) Change image" << endl;
    cout << endl;
    
    bool valid_choice = false;
    int choice = 0;
    while (not valid_choice)
    {
        cout << "Please make a selection" << endl;
        cout << "below. (Any other key to quit)" << endl;
        cout << endl;
        cin >> choice;
        if (choice >= 1 and choice <= 12)
        {
            valid_choice = true;
        }
        else
        {
            valid_choice = true;
            choice = -1;
        }
    } 
    return choice;
}




void sleep()
{
    cout << "Processing..." << endl;
    this_thread::sleep_for(chrono::milliseconds(1000));
}





string savefile()
{
    cout << endl;
    cout << "Save to a new file name: " << endl;
    cout << "(example.bmp)" << endl;
    string new_filename;
    cin >> new_filename;
    return new_filename;
}


//-------------PROCESS_1----------------------------------------------------------------------------------


vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                double distance = sqrt(pow((col - width/2),2)+pow((row - height/2),2));
                double scaling_factor = (height - distance)/height;
                new_image[row][col].red = image[row][col].red*scaling_factor;
                new_image[row][col].green = image[row][col].green*scaling_factor;
                new_image[row][col].blue = image[row][col].blue*scaling_factor;
            }
        }
    return new_image;
}


//-------------PROCESS_2----------------------------------------------------------------------------------


vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                double average_value = (image[row][col].red+image[row][col].green+image[row][col].blue)/3;
                
                if (average_value >= 170)
                {
                    new_image[row][col].red = (int)(255 - (255 - image[row][col].red)*scaling_factor);
                    new_image[row][col].green = (int)(255 - (255 - image[row][col].green)*scaling_factor);
                    new_image[row][col].blue = (int)(255 - (255 - image[row][col].blue)*scaling_factor);
                }
                else if (average_value < 90)
                {
                    new_image[row][col].red = (int)(image[row][col].red*scaling_factor);
                    new_image[row][col].green = (int)(image[row][col].green*scaling_factor);
                    new_image[row][col].blue = (int)(image[row][col].blue*scaling_factor);
                }
                else
                {
                    new_image[row][col].red = image[row][col].red;
                    new_image[row][col].green = image[row][col].green;
                    new_image[row][col].blue = image[row][col].blue;
                }
            }
        }
    return new_image;
}


//-------------PROCESS_3----------------------------------------------------------------------------------


vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                double gray_value = (image[row][col].red+image[row][col].green+image[row][col].blue)/3;
                new_image[row][col].red = gray_value;
                new_image[row][col].green = gray_value;
                new_image[row][col].blue = gray_value;
            }
        }
    return new_image;
}


//-------------PROCESS_4----------------------------------------------------------------------------------


vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(width, vector<Pixel> (height));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                new_image[col][row].red = image[(height-1) - row][col].red;
                new_image[col][row].green = image[(height-1) - row][col].green;
                new_image[col][row].blue = image[(height-1) - row][col].blue;
            }
        }
    return new_image;
}


//-------------PROCESS_5----------------------------------------------------------------------------------


vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)
{
    vector<vector<Pixel>> new_image = image;
    for (int i = 0; i < number; i++)
    {
        new_image = process_4(new_image);
    }
    return new_image;
}


//-------------PROCESS_6----------------------------------------------------------------------------------


vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(y_scale*height, vector<Pixel> (x_scale*width));
    
    for (int row = 0; row < y_scale*height; row++)
        {
            for (int col = 0; col < x_scale*width; col++)
            {
                new_image[row][col].red = image[row/y_scale][col/x_scale].red;
                new_image[row][col].green = image[row/y_scale][col/x_scale].green;
                new_image[row][col].blue = image[row/y_scale][col/x_scale].blue;
            }
        }
    return new_image;
}


//-------------PROCESS_7----------------------------------------------------------------------------------


vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                double gray_value = (image[row][col].red+image[row][col].green+image[row][col].blue)/3;
                
                if (gray_value >= 255/2)
                {
                    new_image[row][col].red = 255;
                    new_image[row][col].green = 255;
                    new_image[row][col].blue = 255;
                }
                else
                {
                    new_image[row][col].red = 0;
                    new_image[row][col].green = 0;
                    new_image[row][col].blue = 0;
                }
            }
        }
    return new_image;
}


//-------------PROCESS_8----------------------------------------------------------------------------------


vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                new_image[row][col].red = (int)(255 - (255 - image[row][col].red)*scaling_factor);
                new_image[row][col].green = (int)(255 - (255 - image[row][col].green)*scaling_factor);
                new_image[row][col].blue = (int)(255 - (255 - image[row][col].blue)*scaling_factor);
            }
        }
    return new_image;
}


//-------------PROCESS_9----------------------------------------------------------------------------------


vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                new_image[row][col].red = image[row][col].red*(scaling_factor);
                new_image[row][col].green = image[row][col].green*(scaling_factor);
                new_image[row][col].blue = image[row][col].blue*(scaling_factor);
            }
        }
    return new_image;
}


//-------------PROCESS_10----------------------------------------------------------------------------------


int maximum(int a, int b, int c)
{
   int max = ( a < b ) ? b : a;
   return (( max < c ) ? c : max);
}

vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
{
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                double max_val = maximum(image[row][col].red, image[row][col].green, image[row][col].blue);
                
                if (image[row][col].red + image[row][col].green + image[row][col].blue >= 550)
                {
                    new_image[row][col].red = 255;
                    new_image[row][col].green = 255;
                    new_image[row][col].blue = 255;
                }
                else if (image[row][col].red + image[row][col].green + image[row][col].blue <= 150)
                {
                    new_image[row][col].red = 0;
                    new_image[row][col].green = 0;
                    new_image[row][col].blue = 0;
                }
                else if (max_val == image[row][col].red)
                {
                    new_image[row][col].red = 255;
                    new_image[row][col].green = 0;
                    new_image[row][col].blue = 0;
                }
                else if (max_val == image[row][col].green)
                {
                    new_image[row][col].red = 0;
                    new_image[row][col].green = 255;
                    new_image[row][col].blue = 0;
                }
                else 
                {
                    new_image[row][col].red = 0;
                    new_image[row][col].green = 0;
                    new_image[row][col].blue = 255;
                }
            }
        }
    return new_image;
}


//-------------MAIN FUNCTIONS----------------------------------------------------------------------------------


void options(int choice, string filename)
{
    vector<vector<Pixel>> subject = read_image(filename);
    
    if (choice == 1)
    {
        cout << "Vignette!" << endl;
        sleep();
        vector<vector<Pixel>> result = process_1(subject);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 2)
    {
        cout << "Clarendon!" << endl;
        cout << "Please enter a scaling factor: ";
        double scaling_factor = 0;
        cin >> scaling_factor;
        sleep();
        vector<vector<Pixel>> result = process_2(subject, scaling_factor);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 3)
    {
        cout << "Gray Scale!" << endl;
        sleep();
        vector<vector<Pixel>> result = process_3(subject);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 4)
    {
        cout << "Rotate 90!" << endl;
        sleep();
        vector<vector<Pixel>> result = process_4(subject);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 5)
    {
        cout << "Rotate 90, x times!" << endl;
        cout << "Please enter the amount " << endl;
        cout << "of times to rotate by 90: ";
        double rotate_factor = 0;
        cin >> rotate_factor;
        sleep();
        vector<vector<Pixel>> result = process_5(subject, rotate_factor);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 6)
    {
        cout << "Enlarge!" << endl;
        cout << "Please enter the enlargment " << endl;
        cout << "factors for each axis: " << endl;
        double x_factor = 0;
        cout << "x factor: ";
        cin >> x_factor;
        cout << endl;
        double y_factor = 0;
        cout << "y factor: ";
        cin >> y_factor;
        sleep();
        vector<vector<Pixel>> result = process_6(subject, x_factor, y_factor);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 7)
    {
        cout << "High Contrast!" << endl;
        sleep();
        vector<vector<Pixel>> result = process_7(subject);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 8)
    {
        cout << "Lighten!" << endl;
        cout << "Please enter % image to " << endl;
        cout << "remain (ex: .1 = 10%): ";
        double scaling_factor = 0;
        cin >> scaling_factor;
        sleep();
        vector<vector<Pixel>> result = process_8(subject, scaling_factor);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 9)
    {
        cout << "Darken!" << endl;
        cout << "Please enter % light to " << endl;
        cout << "remain (ex: .1 = 10%): ";
        double scaling_factor = 0;
        cin >> scaling_factor;
        sleep();
        vector<vector<Pixel>> result = process_9(subject, scaling_factor);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 10)
    {
        cout << "Black, white, red, green, blue!" << endl;
        sleep();
        vector<vector<Pixel>> result = process_10(subject);
        
        write_image(savefile(), result);
        
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == 11)
    {
        cout << endl;
        cout << "Change image!" << endl;
        filename = image_name();
        cout << endl;
        choice = image_menu(filename);
        options(choice, filename);
    }
    
    else if (choice == -1)
    {
        cout << endl;
        cout << "Thank you" << endl;
        cout << "Exiting program" << endl;
        cout << endl;       
        exit(0);
    }
}




int main()
{
    welcome_screen();
    string filename = image_name();
    int choice = image_menu(filename);
    options(choice,filename);
    
    return 0;
}

