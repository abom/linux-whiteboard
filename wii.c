#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <cwiid.h>

cwiid_mesg_callback_t cwiid_callback;

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void print_state(struct cwiid_state *state);

cwiid_err_t err;
void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");
}

//////////////////////////////////////////


cwiid_wiimote_t *wiimote;       /* wiimote handle */
struct cwiid_state state;       /* wiimote state */


extern void infrared_data(int *v);
extern void buttonpress();

int wii_connect(char *mac)
{
	bdaddr_t bdaddr;        /* bluetooth device address */
	unsigned char rpt_mode = 0;

	cwiid_set_err(err);

	str2ba(mac, &bdaddr);

	/* Connect to the wiimote */
        printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
        if (!(wiimote = cwiid_connect(&bdaddr, 0))) {
                fprintf(stderr, "Unable to connect to wiimote\n");
                return 0;
        }
	printf("Connected!!!\n");
        if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
                fprintf(stderr, "Unable to set message callback\n");
		return 0;
        }

        toggle_bit(rpt_mode, CWIID_RPT_IR);
	toggle_bit(rpt_mode, CWIID_RPT_BTN);
        set_rpt_mode(wiimote, rpt_mode);

        cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC);
	return 1;
}




void wii_disconnect()
{
	if (cwiid_disconnect(wiimote)) {
                fprintf(stderr, "Error on wiimote disconnect\n");
        }
}


////////////////////////////////////////////




void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state)
{
	if (cwiid_command(wiimote, CWIID_CMD_LED, led_state)) {
		fprintf(stderr, "Error setting LEDs \n");
	}
}
	
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
	if (cwiid_command(wiimote, CWIID_CMD_RPT_MODE, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}


/* Prototype cwiid_callback with cwiid_callback_t, define it with the actual
 * type - this will cause a compile error (rather than some undefined bizarre
 * behavior) if cwiid_callback_t changes */
/* cwiid_mesg_callback_t has undergone a few changes lately, hopefully this
 * will be the last.  Some programs need to know which messages were received
 * simultaneously (e.g. for correlating accelerometer and IR data), and the
 * sequence number mechanism used previously proved cumbersome, so we just
 * pass an array of messages, all of which were received at the same time.
 * The id is to distinguish between multiple wiimotes using the same callback.
 * */
void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[])
{
	int i, j;
	int valid_source;

	for (i=0; i < mesg_count; i++)
	{
		switch (mesg[i].type) {
		case CWIID_MESG_STATUS:
			printf("Status Report: battery=%d extension=",
			       mesg[i].status_mesg.battery);
			switch (mesg[i].status_mesg.ext_type) {
			case CWIID_EXT_NONE:
				printf("none");
				break;
			case CWIID_EXT_NUNCHUK:
				printf("Nunchuk");
				break;
			case CWIID_EXT_CLASSIC:
				printf("Classic Controller");
				break;
			default:
				printf("Unknown Extension");
				break;
			}
			printf("\n");
			break;
		case CWIID_MESG_BTN:
			printf("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
			buttonpress();
			break;
		case CWIID_MESG_ACC:
			printf("Acc Report: x=%d, y=%d, z=%d\n",
                   mesg[i].acc_mesg.acc[CWIID_X],
			       mesg[i].acc_mesg.acc[CWIID_Y],
			       mesg[i].acc_mesg.acc[CWIID_Z]);
			break;
		case CWIID_MESG_IR:
			valid_source = 0;
			static int v[8];
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				if (mesg[i].ir_mesg.src[j].valid) {
					valid_source = 1;
					//printf("(%d,%d) \n\n", mesg[i].ir_mesg.src[j].pos[CWIID_X],
					//                   mesg[i].ir_mesg.src[j].pos[CWIID_Y]);
					v[j*2] = mesg[i].ir_mesg.src[j].pos[CWIID_X];
					v[j*2+1] = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
				}
				else v[j*2] = v[j*2+1] = 0;
			}
			if (!valid_source) {
				//printf("no sources detected");
			}
			else
				infrared_data(v);

			break;
		case CWIID_MESG_NUNCHUK:
			printf("Nunchuk Report: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
			       "acc.z=%d\n", mesg[i].nunchuk_mesg.buttons,
			       mesg[i].nunchuk_mesg.stick[CWIID_X],
			       mesg[i].nunchuk_mesg.stick[CWIID_Y],
			       mesg[i].nunchuk_mesg.acc[CWIID_X],
			       mesg[i].nunchuk_mesg.acc[CWIID_Y],
			       mesg[i].nunchuk_mesg.acc[CWIID_Z]);
			break;
		case CWIID_MESG_CLASSIC:
			printf("Classic Report: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
			       "l=%d r=%d\n", mesg[i].classic_mesg.buttons,
			       mesg[i].classic_mesg.l_stick[CWIID_X],
			       mesg[i].classic_mesg.l_stick[CWIID_Y],
			       mesg[i].classic_mesg.r_stick[CWIID_X],
			       mesg[i].classic_mesg.r_stick[CWIID_Y],
			       mesg[i].classic_mesg.l, mesg[i].classic_mesg.r);
			break;
		case CWIID_MESG_ERROR:
			if (cwiid_disconnect(wiimote)) {
				fprintf(stderr, "Error on wiimote disconnect\n");
				exit(-1);
			}
			exit(0);
			break;
		default:
			printf("Unknown Report");
			break;
		}
	}
}

