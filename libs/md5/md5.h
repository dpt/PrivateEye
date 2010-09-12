#ifndef MD5_H
#define MD5_H

struct md5_context
{
  unsigned int  buf[4];
  unsigned int  bits[2];
  unsigned char in[64];
};

void md5_init(struct md5_context *ctx);
void md5_update(struct md5_context *ctx, unsigned char *buf, unsigned len);
void md5_final(unsigned char digest[16], struct md5_context *ctx);
void md5_transform(unsigned int buf[4], unsigned int in[16]);

/*  Define CHECK_HARDWARE_PROPERTIES to have main,c verify
    byte order and unsigned int settings.  */
#define CHECK_HARDWARE_PROPERTIES

#endif /* !MD5_H */
