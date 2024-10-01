Specific things:

- Review color selections in Charts
- Switch dependent axes/state labels in Charts to right side
- Remove unneeded console logging
- clean up remaining console log messages
- fiddle with vertical axis/label positioning in the Charts

Intermediate things:

- Add login/sessions to web client and EAd
- Add authentication tokens to REST requests by authenticated session
- Add simple web socket server to EAd (websocketpp?)
   - Use websocket push to tell client to update

Long term wishlist:

- Rewrite DLL in python/go/rust or maybe a typed javascript variant?
   - Merge EAd into DLL for direct access to objects and threadedness
      - Rewrite EAd as a graphql server
         - Rewrite client as a graphql client


