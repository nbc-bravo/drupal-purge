[//]: # ( clear&&curl -s -F input_files[]=@README.md -F from=markdown -F to=html http://c.docverter.com/convert|tail -n+11|head -n-2 )

The purge module clears content from reverse proxy caches like
[Varnish](http://varnish-cache.org/), [Squid](http://www.squid-cache.org/) or
[Nginx](http://nginx.net/) by issuing an HTTP ``PURGE`` request to them. This
allows delivering content updates faster to end users while caching efficiently.

The current stable ``1.x`` branches work in conjunction with the
[cache expiration module](http://drupal.org/project/expire) to act upon events
that are likely to expire URLs from the proxy cache. It will not longer receive
new features, just bugfixes.

## Requirements
*   One or more [reverse proxy caches](http://en.wikipedia.org/wiki/Reverse_proxy)
    like [Varnish](http://varnish-cache.org/) (recommended), [Squid](http://www.squid-cache.org/) or
    [this section in the Varnish chapter](http://nginx.net/) that point to your webserver(s).
    Varnish needs a modification to its configuration file.
    See [this section in the Varnish chapter](http://drupal.org/node/1054886#purge)
    of the Drupal handbook.
*   Squid needs to have purging
    [enabled in its configuration](http://docstore.mik.ua/squid/FAQ-7.html#ss7.5).
*   Nginx needs
    [an extra module and configuration](http://labs.frickle.com/nginx_ngx_cache_purge/).
    See the installation hints below and the included the ``README.txt``, also
    see this [issue #1048000](http://drupal.org/node/1048000) for more
    background info and compiling/installation hints.
*   A cachable version for Drupal 6. This can be an official Drupal 6 release with
    [a patch](http://drupal.org/node/466444) applied or use [Pressflow](http://pressflow.org/),
    a cachable friendly fork of Drupal. Drupal 7 works out of the box.
*   PHP with [curl](http://php.net/manual/en/book.curl.php) enabled. The ``1.x``
    releases of Purge uses curl for issuing the http PURGE requests.

## Installation
*   Unpack, place and enable just like any other module.
*   Navigate to ``Administration`` -> ``Site configuration`` -> ``Purge settings``.
*   Set your proxy URL(s) like ``http://localhost`` or
    ``http://192.168.1.23:8080 http://192.168.2.34:8080``.
*   If you are using nginx you need to specify the purge path and the get method
    in your proxy setting like this: ``http://192.168.1.76:8080/purge?purge_method=get``
*   If you are using the Acquia Cloud we recommend you use the platform specific
    module [Acquia Purge](http://drupal.org/project/acquia_purge) instead.
*   Optional: Install [Rules](http://drupal.org/project/rules) for advanced
    cache clearing scenarios or [Drush](http://drupal.org/project/drush) for
    command line purging. Both are supported through the expire module.

## Q&A

###### How do I know if its working?
Purge reports errors to watchdog. Also when running "varnishlog" on the
proxy your should see PURGE requests scrolling by when you (for instance)
update an existing node.

###### How can I test this more efficiently?
The expire module has drush support so you can issue purge commands from
the command line. See [#1054584](http://drupal.org/node/1054584)
You can also test if your proxy is configured correctly by issuing a curl
command in a shell on any machine in the access list of your proxy:
curl -X PURGE -H "Host: example.com" http://192.168.1.23/node/2

###### Why choose this over the [Varnish module](http://drupal.org/project/varnish)?
Purge just issues purge requests to your proxy server(s) over standard http
on every url the expire module detects. It requires modification of your
Varnish configuration file.

The varnish module has more internal logic to purge your Varnish cache
completely, which can be more disruptive then the expire module integration it
also offers. It uses a terminal interface to communicate to varnish instead of
http. This allows for more features but also hands over full control over the
varnish instance to any machine having network access to it. (This is a
limitation of Varnish.) Also firewall or other security policies could pose a
problem. It does not require modification of your config file. If you have the
choice Varnish module is probably your best bet but Purge might help you out in
places where Varnish module is not an option.

###### What happened to support for Acquia Cloud platforms?
Although this was supported in earlier versions of the ``1.x`` series, we've
dropped this in favor of the [Acquia Purge](http://drupal.org/project/acquia_purge)
module which gives a smoother and more integrated experience.
