https://t.me/binance_api_english

Base Endpoints:
https://api.binance.com
https://api1.binance.com
https://api2.binance.com
https://api3.binance.com

HTTP Return Codes
HTTP 4XX return codes are used for malformed requests; the issue is on the sender's side.
HTTP 403 return code is used when the WAF Limit (Web Application Firewall) has been violated.
HTTP 429 return code is used when breaking a request rate limit.
HTTP 418 return code is used when an IP has been auto-banned for continuing to send requests after receiving 429 codes.
HTTP 5XX return codes are used for internal errors; the issue is on Binance's side. It is important to NOT treat this as a failure operation; the execution status is UNKNOWN and could have been a success.

ERROR CODES
https://binance-docs.github.io/apidocs/spot/en/#error-codes


Max 5 messages per second
A single connection can listen to a maximum of 1024 streams.

Base endpoints:
wss://stream.binance.com:9443


Open a stream to wss://stream.binance.com:9443/ws/bnbbtc@depth

Get a depth snapshot from https://api.binance.com/api/v3/depth?symbol=BNBBTC&limit=1000
