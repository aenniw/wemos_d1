#include <web_service.h>

void WiFi_event(WiFiEvent_t event) {
    switch (event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
#ifdef __DEBUG__
            Serial.printf("Got IP:%s\n", WiFi.localIP().toString().c_str());
#endif
            if (MDNS.begin(DNS_HOSTNAME))
                MDNS.addService("http", "tcp", 80);
#ifdef __DEBUG__
        else
            Serial.println("Error setting up MDNS responder!");
#endif
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
#ifdef __DEBUG__
            Serial.println("WiFi lost connection");
#endif
            break;
        case WIFI_EVENT_STAMODE_CONNECTED:
#ifdef __DEBUG__
            Serial.println("WiFi accepted");
#endif
            break;
        case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
            break;
        case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
            break;
        case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
#ifdef __DEBUG__
            Serial.println("Client connected");
#endif
            break;
        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
#ifdef __DEBUG__
            Serial.println("Client disconnected");
#endif
            break;
        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            break;
        case WIFI_EVENT_MAX:
            break;
        case WIFI_EVENT_MODE_CHANGE:
            break;
    }
}

WebService::WebService(const char *user, const char *pass) {
    WiFi.onEvent(WiFi_event);
    //dns_server = new DNSServer();
    //dns_server->setErrorReplyCode(DNSReplyCode::NoError);
    //dns_server->start(53, DNS_HOSTNAME_AP, IPAddress(192, 168, 4, 1));
    acc = new String(user);
    passwd = new String(pass);
    web_server = new ESP8266WebServer(80);
    const char *headerkeys[] = {"Cookie"};
    web_server->collectHeaders(headerkeys, 1);
    web_server->onNotFound([this]() { on_not_found(); });
    web_server->on("/login", [=]() {
        if (!valid_credentials()) {
            on_invalid_credentials();
        } else if (HTTP_GET == web_server->method()) {
            String header("HTTP/1.1 301 OK\r\nSet-Cookie: LOGINID=");
            header += ESP.getChipId();
            header += "\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
            web_server->sendContent(header);
        } else
            on_not_found();
    });
    web_server->on("/logout", [=]() {
        if (!valid_credentials())
            return on_invalid_credentials();
        if (HTTP_GET == web_server->method()) {
            web_server->sendContent(
                    "HTTP/1.1 301 OK\r\nSet-Cookie: LOGINID=0\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n");
        } else
            on_not_found();
    });
    web_server->begin();
}

void WebService::on_not_found() {
#ifdef __DEBUG__
    String message = "Not Found\n\nURI: ";
    message += web_server->uri();
    message += "\nMethod: ";
    message += (web_server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nHeaders: ";
    message += web_server->headers();
    message += "\n";
    for (uint8_t i = 0; i < web_server->headers(); i++) {
        message +=
                " " + web_server->headerName(i) + ": " + web_server->header(i) + "\n";
    }
    message += "\nArguments: " + web_server->args();
    message += "\n";
    for (uint8_t i = 0; i < web_server->args(); i++) {
        message += " " + web_server->argName(i) + ": " + web_server->arg(i) + "\n";
    }
    web_server->send(404, RESP_TEXT, message);
#else
    web_server->send(404, RESP_TEXT, "Not found");
#endif
}

void WebService::on_invalid_credentials() {
    File file = SPIFFS.open("/login.html", "r");
    if (file) {
        web_server->streamFile(file, RESP_HTML);
        file.close();
    } else {
        web_server->requestAuthentication();
    }
}

bool WebService::valid_credentials() {
    if (acc->length() == 0 || passwd->length() == 0)
        return true;
    if (web_server->hasHeader("Cookie")) {
        String valid_cookie("LOGINID=");
        valid_cookie += ESP.getChipId();
        if (web_server->header("Cookie").indexOf(valid_cookie) != -1) {
            return true;
        }
    }
    return web_server->authenticate(acc->c_str(), passwd->c_str());
}

void WebService::add_handler(const char *uri, HTTPMethod method,
                             const char *resp_type,
                             WebServiceFunction handler) {
    web_server->on(uri, [=]() {
#ifdef __DEBUG__
        Serial.printf("Recieved on %s\n", uri);
#endif
        if (method == HTTP_ANY || method == web_server->method()) {
            web_server->send(200, resp_type, handler(web_server->arg("plain")));
        } else
            on_not_found();
    });
}

void WebService::add_handler_auth(const char *uri, HTTPMethod method,
                                  const char *resp_type,
                                  WebServiceFunction handler) {
    web_server->on(uri, [=]() {
#ifdef __DEBUG__
        Serial.printf("Recieved on %s\n", uri);
#endif
        if (!valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            web_server->send(200, resp_type, handler(web_server->arg("plain")));
        } else
            on_not_found();
    });
}

void WebService::add_handler_file(const char *uri, HTTPMethod method,
                                  const char *resp_type,
                                  const char *file_name) {
    web_server->on(uri, [=]() {
#ifdef __DEBUG__
        Serial.printf("Recieved on %s\n", uri);
#endif
        if (method == HTTP_ANY || method == web_server->method()) {
            File file = SPIFFS.open(file_name, "r");
            web_server->streamFile(file, resp_type);
            file.close();
        } else
            on_not_found();
    });
}

void WebService::add_handler_file_auth(const char *uri, HTTPMethod method,
                                       const char *resp_type,
                                       const char *file_name) {
    web_server->on(uri, [=]() {
#ifdef __DEBUG__
        Serial.printf("Recieved on %s\n", uri);
#endif
        if (!valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            File file = SPIFFS.open(file_name, "r");
            web_server->streamFile(file, resp_type);
            file.close();
        } else
            on_not_found();
    });
}

void WebService::cycle_routine() {
    //dns_server->processNextRequest();
    web_server->handleClient();
}
