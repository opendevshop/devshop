
# Defaut Backend: matches apache.
backend default {
    .host = "127.0.0.1";
    .port = "<?php print $http_port;?>";
    .max_connections = 250;
    .connect_timeout = 300s;
    .first_byte_timeout = 300s;
    .between_bytes_timeout = 300s;
}

# Only allow purge requests from localhost.
acl purge {
    "localhost";
    "127.0.0.1";
}


# Varnish Custom Config
# From DigitalOcean:
# https://www.digitalocean.com/community/tutorials/how-to-configure-varnish-for-drupal-with-apache-on-debian-and-ubuntu

sub vcl_recv {

    # A great functionality of Varnish is to check
    # your web server's health and serve stale pages
    # if necessary.
    # In case of web server lag, let's return the
    # request with stale content.

    if (req.backend.healthy)
    {
        set req.grace = 60s;
    }
    else
    {
        set req.grace = 30m;
    }

    # Modify (remove) progress.js request parameters.

    if (req.url ~ "^/misc/progress\.js\?[0-9]+$")
    {
        set req.url = "/misc/progress.js";
    }

    # Modify HTTP X-Forwarded-For header.
    # This will replace Varnish's IP with actual client's.

    remove req.http.X-Forwarded-For;
    set    req.http.X-Forwarded-For = client.ip;

    # Check if request is allowed to invoke cache purge.

    if (req.request == "PURGE")
    {
        if (!client.ip ~ purge)
        {
            # Return Error 405 if not allowed.
            error 405 "Forbidden - Not allowed.";
        }
        return (lookup);
    }

    # Verify HTTP request methods.

    if (req.request != "GET"    && req.request != "HEAD" &&
        req.request != "PUT"    && req.request != "POST" &&
        req.request != "TRACE"  && req.request != "OPTIONS" &&
        req.request != "DELETE" && req.request != "PURGE")
    {
            return (pipe);
    }

    # Handling of different encoding types.

    if (req.http.Accept-Encoding)
    {
        if (req.url ~ "\.(jpg|png|gif|gz|tgz|bz2|tbz|mp3|ogg)$")
        {
            remove req.http.Accept-Encoding;
        }
        elsif (req.http.Accept-Encoding ~ "gzip")
        {
            set req.http.Accept-Encoding = "gzip";
        }
        elsif (req.http.Accept-Encoding ~ "deflate")
        {
            set req.http.Accept-Encoding = "deflate";
        }
        else
        {
            remove req.http.Accept-Encoding;
        }
    }

    # Force look-up if request is a no-cache request.
    if (req.http.Cache-Control ~ "no-cache")
    {
        return (pass);
    }

    # Do not allow outside access to cron.php or install.php. Depending on your access to the server, you might want to comment-out this block of code for development.
    if (req.url ~ "^/(cron|install)\.php$" && !client.ip ~ internal)
    {
        # Throw error directly:
        error 404 "Page not found.";
        # Or;
        # Use a custom error page on path /error-404.
        # set req.url = "/error-404";
    }

    # Remove certain cookies.
    set req.http.Cookie = regsuball(req.http.Cookie, "has_js=[^;]+(; )?", "");
    set req.http.Cookie = regsuball(req.http.Cookie, "Drupal.toolbar.collapsed=[^;]+(; )?", "");
    set req.http.Cookie = regsuball(req.http.Cookie, "__utm.=[^;]+(; )?", "");
    if (req.http.cookie ~ "^ *$")
    {
        unset req.http.cookie;
    }

    # Cache static content of themes.
    if (req.url ~ "^/themes/" && req.url ~ ".(css|js|png|gif|jp(e)?g)")
    {
        unset req.http.cookie;
    }

    # Do not cache these URL paths.
    if (req.url ~ "^/status\.php$" ||
        req.url ~ "^/update\.php$" ||
        req.url ~ "^/ooyala/ping$" ||
        req.url ~ "^/admin"        ||
        req.url ~ "^/admin/.*$"    ||
        req.url ~ "^/user"         ||
        req.url ~ "^/user/.*$"     ||
        req.url ~ "^/users/.*$"    ||
        req.url ~ "^/info/.*$"     ||
        req.url ~ "^/flag/.*$"     ||
        req.url ~ "^.*/ajax/.*$"   ||
        req.url ~ "^.*/ahah/.*$")
    {
        return (pass);
    }

    # Cache the following file types.
    if (req.url ~ "(?i)\.(png|gif|jpeg|jpg|ico|swf|css|js|html|htm)(\?[a-z0-9]+)?$")
    {
        unset req.http.Cookie;
    }

    # !! Do not cache application area
    if (req.url ~ "(^/app.php|^/app_dev.php|^)/([a-z]{2})/(payment|order|booking|media|autocomplete|monitor).*")
    {
        return (pass);
    }

    # !! Do not cache admin area
    if (req.url ~ "(^/app.php|^/app_dev.php|^)/admin" || req.url ~ "(^/app.php|^/app_dev.php|^)/(([a-z]{2})/admin)")
    {
        return (pass);
    }

    # !! Do not cache security area
    if (req.url ~ "(^/app.php|^/app_dev.php|^)/(([a-z]{2}/|)(login|logout|login_check).*)")
    {
        return (pass);
    }

    # Do not cache editor logged-in user sessions
    if (req.http.Cookie ~ "(sonata_page_is_editor)")
    {
        return (pass);
    }

    return (lookup);
}

sub vcl_hit {
    if (req.request == "PURGE")
    {
        purge;
        error 200 "Purged.";
    }
}

sub vcl_miss {
    if (req.request == "PURGE")
    {
        purge;
        error 200 "Purged.";
    }
}