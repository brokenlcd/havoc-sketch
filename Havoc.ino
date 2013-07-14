/*
 * Copyright (c) 2013 by Kyle Isom <kyle@tyrfingr.is>.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


#define READBYTE(x)     analogRead((x)) >> 2
// collect samples for ten seconds during init.
#define INIT_TIME       10000
// baud rate for serial output
#define BAUDRATE        9600
#define RNGPUT          1
// number of values in the sample array
#define NSAMP           256

int median0;    // median value for ANALOG0
int median1;    // median value for ANALOG1

const int rbg0 = 0;   // Pin for RBG0.
const int rbg1 = 1;   // Pin for RBG1.


void
Fault(char msg[])
{
        Serial.write(0);
        Serial.write(0x20);
        Serial.write(msg);
        Serial.write(0x0a);
}


/*
 * readRBG0 and readRBG1 read a bit from their respective RBG; if
 * this is above the median, the function returns a -1 to indicate
 * the value is to be discarded. If the value is below the median,
 * a 0 is returned. If the value is above the median, a 1 is returned.
 */
int
readRBG0()
{
        byte b = READBYTE(rbg0);
        if (b == median0)
                return -1;
        else if (b < median0)
                return 0;
        else if (b > median0)
                return 1;
        else
                Fault("RBG0 invalid value");
        return -1;
}


int
readRBG1()
{
        byte b = READBYTE(rbg1);
        if (b == median1)
                return -1;
        else if (b < median1)
                return 0;
        else if (b > median1)
                return 1;
        else
                Fault(" RBG1 invalid value");
        return -1;
}


/*
 * setup collects samples for 10 seconds, taking the median value
 * from each for calibration.
 */
void
setup()
{
        Serial.begin(BAUDRATE);
        int i = 0;
        unsigned int samp0[NSAMP];
        unsigned int samp1[NSAMP];

        for (i = 0; i < NSAMP; i++) {
                samp0[i] = 0;
                samp1[i] = 0;
        }

        unsigned long start = millis();
        unsigned long stop = start + INIT_TIME;
        int sum0 = 0;
        int sum1 = 0;
        byte half0, half1;
        byte cal0 = 0;
        byte cal1 = 0;

        while (millis() < stop) {
                samp0[READBYTE(rbg0)]++;
                samp1[READBYTE(rbg1)]++;
        }

        median0 = sum0 = 0;
        median1 = sum1 = 0;
        for (i = 0; i < NSAMP; i++) {
                sum0 += samp0[i];
                sum1 += samp1[i];
        }
        half0 = sum0 >> 1;
        half1 = sum1 >> 1;
        sum0 = sum1 = 0;
        cal0 = cal1 = 0;
        for (i = 0; i < NSAMP; i++) {
                if (!cal0) {
                        sum0 += samp0[i];
                        if (sum0 > half0)
                                cal0 = i;
                }

                if (!cal1) {
                        sum1 += samp1[i];
                        if (sum1 > half1)
                                cal1 = i;
                }
                if (cal0 && cal1)
                        break;
        }
        median0 = samp0[cal0];
        median1 = samp0[cal1];
}


/*
 * readRBG compares the two values coming from the RBGs. If either bit is
 * a failure or the two bits are equal, return a failure. This failure
 * does indicate a fault in the RBG, but rather that the result was discarded
 * and a new result should be taken.
 */
int
readRBG()
{
        byte b0 = readRBG0();
        byte b1 = readRBG1();

        if (b0 != -1)
        if (b1 != -1)
        if (b0 != b1)
                return b0;
        return -1;
}


/*
 * loop begins once Havoc has been calibrated. It builds a byte at a time;
 * once a full byte has been built, it will be written out to the serial
 * port.
 */
void
loop()
{
        static byte rval = 0;   // The random byte being built.
        static int n = 0;       // The current bit position in the byte.
        byte rbit = readRBG();

        if (rbit != -1) {
                rval &= (rbit << n);
                n++;
                if (n == 8) {
                        Serial.write(RNGPUT);
                        Serial.write(0x20);
                        Serial.write(rval);
                        Serial.write(0x0a);
                        rval = 0;
                        n = 0;
                }
        }
}
