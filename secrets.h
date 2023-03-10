#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "123"                         //change this
 
//const char WIFI_SSID[] = "Jihwan";               //change this
//const char WIFI_PASSWORD[] = "11111111";           //change this
const char AWS_IOT_ENDPOINT[] = "a2jsf49wm15nzt-ats.iot.ap-northeast-1.amazonaws.com";       //change this
 
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                               //change this
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAIdmBKQ9UwOXCV1Ad4c5u10wQ3L0MA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMjEwMTQxMjQ5
MjFaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaJNkUgJ9QMvt/Hdc8
qoHDVMxJAF1ZBhy2ulqXZ9iLyG8tCWGKm5GFIGqbGB1TdAYWllycgJtrnXu5R3Xq
Or6LF6xC6tO9phVuZBdHpezfpSAEvn9ZJEC9XfV9735e+vTKlXOK4hCRW7TyBoZ7
Iclc88UfgreqRu4eFQ+PUOgtMFPVUUmRdmZQHyv6+8cUUtcGXfU86hzv+/EhKhIO
uFDRYCiVT1uFowr2npKCQmAqxhH6YMv4obKaYvXceERc/VpphzrLkdiBYyRSwqjM
OjLzxb14k1ws71238XE6RewhZmXED57crDVYXgFbg9coj3W3PiRevMk946zpZUfd
8+dTAgMBAAGjYDBeMB8GA1UdIwQYMBaAFEaQnr1L4TK3uvg8TZCyYzlaEYaaMB0G
A1UdDgQWBBQs9YZnQA0pzKUu6mx63baRjwsXCTAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAmcW6iTjg4K5YrUlSNME3jcll
2UcZIkIKcnpd0OqKixAMruYwkTUlxQrikzLmGa3SyqRnJvjnP9CuiA57Co+X7woq
Ib49bcskO7L+ZDu2B/kjdxt0etTop9ZFh6P5CJ78lwX4cfGjIf5/uMpPaHpLiluf
umSp4NTYmyT6Yn5TQm8gqlAHunqZ//bNbBGtq3Mb0gDrZSwJbDQ4iN25cd4WhFV3
n30C9gKajTYsl4IdqQpGVnrzP1hYuPf2U219tYho2RZzwED5d5hxORvFsPo62upN
yiGAOqTpRMPsP+ilqwRScRzUpMu256d1WmJOimdQ+O17XViIFmO8ZfP2Qtg21A==
-----END CERTIFICATE-----



 
 
)KEY";
 
// Device Private Key                                               //change this
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA2iTZFICfUDL7fx3XPKqBw1TMSQBdWQYctrpal2fYi8hvLQlh
ipuRhSBqmxgdU3QGFpZcnICba517uUd16jq+ixesQurTvaYVbmQXR6Xs36UgBL5/
WSRAvV31fe9+Xvr0ypVziuIQkVu08gaGeyHJXPPFH4K3qkbuHhUPj1DoLTBT1VFJ
kXZmUB8r+vvHFFLXBl31POoc7/vxISoSDrhQ0WAolU9bhaMK9p6SgkJgKsYR+mDL
+KGymmL13HhEXP1aaYc6y5HYgWMkUsKozDoy88W9eJNcLO9dt/FxOkXsIWZlxA+e
3Kw1WF4BW4PXKI91tz4kXrzJPeOs6WVH3fPnUwIDAQABAoIBADR5irKw4iHzsaX5
cq5IQeDQBDhHWy0wGDYoi+RB+cheVcgaqpkiJRZGHv7iBSPvFAvY7bxD/58aigme
1BGdQfrJngmW3YqQajVc5HC6zElcOmCJxq6V1vD2qjg6JUcJzVryKpItIVkmG1fh
E/aIwRyYE0IqOq4+U8duv4h1KPS9aqtFFM/rAYAH2DFylfW7i4yLKmgAbgCh2zQe
dSvhcsMyzKFSByOUZUreNwJ/OvJKb0bniRGG/ZAebYLOu7/a5uUOwgzgRnQtv+C4
x5R+vZIoF8+1pgEGCpAa0x8xD2Ff8/gwcjq1VJgU3oBvSwTqhX5WdoTgdy8Dj2aM
ILFWPjkCgYEA7Y/7n1qOCKiq72/BoM782LbqEL285EWZPjvUg2GaBzEnTbs3tN1f
caE/+VrGEO+Nq8S0/g48TrmTm4homfbCPEpaCptyBIsdbWbTfc2W438aCPcISVZe
/vqo19eaQJE0QUauDMIuLBSw4zVOMdb+Gqi6NlXlmgfFuRkfpDZWxjcCgYEA6xMM
TEp+iGCEx6eEqU915cGdMBm8xQe6DGL6HRUZgMcOMvnxD5SjptsaAkEnnlY8KEln
aIFQ1KtFkz2RIEwsfadPPRZZOclJ0mwHdLJgcGvsctMRvoAso9DLxSXe5LMSHVev
UFntVHlzXTscoZBrEyvPcPGeqD7TMyiA48G+GcUCgYBwb5ceAtP6kSnmDTTNkWnm
gBwkNEk0mSRAajzYudcW1FLP3W4uMiA8PZ9zu1zHIzogNAedvssYT4jnMhaf2ERy
GJapMo0yTU1uPB6ZCjcCEWQXHiRT4Ycwkx5amfat3Iuo4XT7IOSNhHV78/zAZgr6
PyVzFhCHaGpSguuQ4w8K3QKBgHOn54ERlbebZTil3ss+YQ8tdxzXTNzTtGGrwMrW
Orn1NbM27bdwtiO94MoDLG7mlqprkwkDtrUrnwuWWvS9BZa83mt7t2KK+lsg83oi
2Pf24VqNJknH3i6q59aTj1qOD/eH0KWD2RSavHu+PZPv7f58q48yEZFNd7PP1bo4
9ELRAoGBAJYyLYlPFDQau/USheacQfPz7mCsaBBfeUh1kPayYeuz8TzTCia1R6Qn
wZCIK09L+JcrbwDFqFqxLuBZMo8SS2/PjUssDCIR7lwr28iGxhqqbHVnJ59zyncF
kTqW6YtICne2eC5TW5hxG7XHGMpi4TBtdXzCseSWbQ5/CNg6UJhl
-----END RSA PRIVATE KEY-----




 
 
)KEY";
