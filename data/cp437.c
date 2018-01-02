#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * The triggering character for UNICODE is: 
 *
 *  226	E2	
	Greek upper case letter gamma 
 **/

#define BUFSIZE 32767 

int main()
{
    bool unicode_on = false;
    int unicode_byte_index = 0;
    int rd = 0;
    int i = 0;
    uint16_t ubyte = 0;
    unsigned char buffer[BUFSIZE];

    while (!rd) {
        rd = read(0, &buffer, BUFSIZE);
        fprintf(stderr, "rd = %d\n", rd);
        for (i = 0; i < rd ; i++) {
            if (unicode_on) {
                    unicode_byte_index ++;
                    switch(unicode_byte_index) {
                        case 1:
                            //ubyte = buffer[i];
                            ubyte = (buffer[i] << 8);
                            break;
                        case 2:
                            /* FIXME: not endian safe ! */
                            //ubyte = ubyte + (buffer[i] << 8);
                            ubyte += buffer[i]; 
                            break;
                        default:
                            fprintf(stderr, "+++ Unicode character has bad length! = %d\n", unicode_byte_index);
                            break;
                        }
                    if (unicode_byte_index == 2) {
                        fprintf(stderr, "+++ ubyte = 0x%04x\n", ubyte);
                        switch (ubyte) {
                            case 0x9688:
                                /* 'FULL BLOCK' (U+2588) */
                                putchar(0xDB);
                                break;
                            case 0x9597:
                                /* 'BOX DRAWINGS DOUBLE DOWN AND LEFT' (U+2557) */
                                putchar(0xBB);
                                break;
                            case 0x9591:
                                /* 'BOX DRAWINGS DOUBLE VERTICAL' (U+2551) */
                                putchar(0xBA);
                                break;
                            case 0x9594:
                                /* 'BOX DRAWINGS DOUBLE DOWN AND RIGHT' (U+2554) */
                                putchar(0xC9);
                                break;
                            case 0x9590:
                                /* 'BOX DRAWINGS DOUBLE HORIZONTAL' (U+2550) */
                                putchar(0xCD);
                                break;
                            case 0x959d:
                                /* 'BOX DRAWINGS DOUBLE UP AND LEFT' (U+255D) */
                                putchar(0xBC);
                                break; 
                            case 0x959a:
                                /* 'BOX DRAWINGS DOUBLE UP AND RIGHT' (U+255A) */
                                putchar(0xC8);
                                break; 
                            case 0x948c:
                                /* 'BOX DRAWINGS LIGHT DOWN AND RIGHT' (U+250C)*/
                                putchar(0xDA);
                                break; 
                            case 0x9480:
                                /* 'BOX DRAWINGS LIGHT HORIZONTAL' (U+2500) */
                                putchar(0xC4);
                                break; 
                            case 0x9490:
                                /* 'BOX DRAWINGS LIGHT DOWN AND LEFT' (U+2510) */
                                putchar(0xBF);
                                break; 
                            case 0x9482:
                                /* 'BOX DRAWINGS LIGHT VERTICAL' (U+2502) */
                                putchar(0xB3);
                                break; 
                            case 0x9692:
                                /* 'MEDIUM SHADE' (U+2592) */
                                putchar(0xB1);
                                break; 
                            case 0x9691:
                                /* 'LIGHT SHADE' (U+2591) */
                                putchar(0xB0);
                                break; 
                            case 0x9693:
                                /* 'DARK SHADE' (U+2593) */
                                putchar(0xB2);
                                break; 
                            case 0x9494:
                                /* 'BOX DRAWINGS LIGHT UP AND RIGHT' (U+2514)*/
                                putchar(0xC0);
                                break; 
                            case 0x9498:
                                /* 'BOX DRAWINGS LIGHT UP AND LEFT' (U+2518)*/
                                putchar(0xD9);
                                break; 
                            case 0x949c:
                                /* 'BOX DRAWINGS LIGHT VERTICAL AND RIGHT' (U+251C) */
                                putchar(0xC3);
                                break; 
                            case 0x94ac:
                                /* 'BOX DRAWINGS LIGHT DOWN AND HORIZONTAL' (U+252C) */
                                putchar(0xC2);
                                break; 
                            case 0x94a4:
                                /* 'BOX DRAWINGS LIGHT VERTICAL AND LEFT' (U+2524) */
                                putchar(0xB4);
                                break; 
                            case 0x9680:
                                /* 'UPPER HALF BLOCK' (U+2580) */
                                putchar(0xDF);
                                break;
                            case 0x9684:
                                /* 'LOWER HALF BLOCK' (U+2584) */
                                putchar(0xDC);
                                break;
                            default:
                                fprintf(stderr, "UNHANDLED UBYTE VALUE = e2%04x [U+%04X]\n", ubyte, ubyte - 0x7040 - 0x40);
                                exit(1);
                                break;
                            }

                        unicode_on = false;
                        unicode_byte_index = 0;
                        ubyte = 0;
                        }
            } else {

                switch(buffer[i]) {
                case 0xE2:
                    /* some freaky unicode magic symbol */
                    fprintf(stderr, "\n[UNICODE=0x%02x]\n", buffer[i]);
                    unicode_on = true;
                    unicode_byte_index = 0;
                    ubyte = 0;
                    break;
                default:
                    if (buffer[i] >= 128) {
                        fprintf(stderr,
                                "+ Unicode error, buffer[%d] = 0x%02x!\n",
                                i, buffer[i]);
                        exit(1);
                    }
                    //fprintf(stderr, "[0x%02x]", buffer[i]);
                    putchar(buffer[i]);
                    break;
                }
            }

        }
    }

}
