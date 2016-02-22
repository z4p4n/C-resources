#ifndef ZPN_SMTP_H
#define ZPN_SMTP_H

#define ZPN_SMTP_PORT 25
#define ZPN_SMTP_PORT_OVH 587
#define ZPN_SMTP_OVH "ns0.ovh.net"
#define ZPN_BBOX "smtp.bbox.fr"

#include <stdint.h>

int zpn_send_mail (const char *host, uint16_t port, 
                   const char *receptor, const char *sender, const char *name,
		   const char *subject, const char *data, 
		   const char *login, const char *password);

#endif
