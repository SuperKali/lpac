#include "at.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <euicc/interface.h>
#include <euicc/hexutil.h>

#define AT_BUFFER_SIZE 20480
static FILE *fuart;
static int logic_channel = 1;
static char *buffer;

static int at_expect(char **response, const char *expected)
{
    char line[1024];
    int ret = -1;
    if (response)
        *response = NULL;
    while (fgets(line, sizeof(line), fuart) != NULL) {
        line[strcspn(line, "\r\n")] = 0;
        if (strcmp(line, "OK") == 0) {
            ret = 0;
            break;
        } else if (strcmp(line, "ERROR") == 0) {
            ret = -1;
            break;
        } else if (strncmp(line, "+CME ERROR:", 11) == 0) {
            ret = -1;
            break;
        }
        if (expected && strncmp(line, expected, strlen(expected)) == 0) {
            const char *rest = line + strlen(expected);
            while (*rest == ' ' || *rest == ':') rest++;
            if (response)
                *response = strdup(rest);
        }
    }
    return ret;
}

static int apdu_interface_connect(struct euicc_ctx *ctx)
{
    const char *device = getenv("AT_DEVICE") ? getenv("AT_DEVICE") : "/dev/ttyUSB0";
    fuart = fopen(device, "r+");
    if (fuart == NULL)
        return -1;
    fprintf(fuart, "ATE0\r\n");
    if (at_expect(NULL, NULL) < 0)
        return -1;
    fprintf(fuart, "AT+CCHO=?\r\n");
    if (at_expect(NULL, NULL) < 0)
        return -1;
    fprintf(fuart, "AT+CCHC=?\r\n");
    if (at_expect(NULL, NULL) < 0)
        return -1;
    fprintf(fuart, "AT+CGLA=?\r\n");
    if (at_expect(NULL, NULL) < 0)
        return -1;
    return 0;
}

static void apdu_interface_disconnect(struct euicc_ctx *ctx)
{
    if (fuart) {
        fclose(fuart);
        fuart = NULL;
    }
}

static int apdu_interface_transmit(struct euicc_ctx *ctx, uint8_t **rx, uint32_t *rx_len, const uint8_t *tx, uint32_t tx_len)
{
    int fret = 0;
    int ret;
    char *response = NULL;
    char *hexstr = NULL;
    *rx = NULL;
    *rx_len = 0;
    fprintf(fuart, "AT+CGLA=%d,%u,\"", logic_channel, tx_len * 2);
    for (uint32_t i = 0; i < tx_len; i++)
        fprintf(fuart, "%02X", (uint8_t)(tx[i] & 0xFF));
    fprintf(fuart, "\"\r\n");
    if (at_expect(&response, "+CGLA:") < 0)
        goto err;
    if (response == NULL)
        goto err;
    {
        char *p = response;
        while (*p == ' ' || *p == ':') p++;
        strtok(p, ",");
        hexstr = strtok(NULL, ",");
        if (!hexstr)
            goto err;
        while (*hexstr == ' ' || *hexstr == '"') hexstr++;
        char *endq = strchr(hexstr, '"');
        if (endq) *endq = '\0';
        *rx_len = strlen(hexstr) / 2;
        *rx = malloc(*rx_len);
        if (!*rx)
            goto err;
        ret = euicc_hexutil_hex2bin_r(*rx, *rx_len, hexstr, strlen(hexstr));
        if (ret < 0)
            goto err;
        *rx_len = ret;
    }
    goto exit;
err:
    fret = -1;
    if (*rx) {
        free(*rx);
        *rx = NULL;
    }
    *rx_len = 0;
exit:
    free(response);
    return fret;
}

static int apdu_interface_logic_channel_open(struct euicc_ctx *ctx, const uint8_t *aid, uint8_t aid_len)
{
    char *response = NULL;
    for (int i = 1; i <= 4; i++) {
        fprintf(fuart, "AT+CCHC=%d\r\n", i);
        at_expect(NULL, NULL);
    }
    fprintf(fuart, "AT+CCHO=\"");
    for (int i = 0; i < aid_len; i++)
        fprintf(fuart, "%02X", (uint8_t)(aid[i] & 0xFF));
    fprintf(fuart, "\"\r\n");
    if (at_expect(&response, "+CCHO:") < 0) {
        free(response);
        return -1;
    }
    free(response);
    return 1;
}

static void apdu_interface_logic_channel_close(struct euicc_ctx *ctx, uint8_t channel)
{
    fprintf(fuart, "AT+CCHC=%d\r\n", logic_channel);
    at_expect(NULL, NULL);
}

static int libapduinterface_init(struct euicc_apdu_interface *ifstruct)
{
    memset(ifstruct, 0, sizeof(struct euicc_apdu_interface));
    ifstruct->connect = apdu_interface_connect;
    ifstruct->disconnect = apdu_interface_disconnect;
    ifstruct->logic_channel_open = apdu_interface_logic_channel_open;
    ifstruct->logic_channel_close = apdu_interface_logic_channel_close;
    ifstruct->transmit = apdu_interface_transmit;
    buffer = malloc(AT_BUFFER_SIZE);
    if (!buffer)
        return -1;
    return 0;
}

static int libapduinterface_main(int argc, char **argv)
{
    return 0;
}

static void libapduinterface_fini(struct euicc_apdu_interface *ifstruct)
{
    free(buffer);
}

const struct euicc_driver driver_apdu_at = {
    .type = DRIVER_APDU,
    .name = "at",
    .init = (int (*)(void *))libapduinterface_init,
    .main = libapduinterface_main,
    .fini = (void (*)(void *))libapduinterface_fini,
};
