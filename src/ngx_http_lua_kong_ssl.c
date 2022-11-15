/**
 * Copyright 2019-2022 Kong Inc.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "ngx_http_lua_kong_common.h"

/*
 * disables session reuse for the current TLS connection, must be called
 * in ssl_certby_lua* phase
 */

const char *
ngx_http_lua_kong_ffi_disable_session_reuse(ngx_http_request_t *r)
{
#if (NGX_SSL)
    return ngx_lua_kong_ssl_disable_session_reuse(r->connection);
#else
    return "TLS support is not enabled in Nginx build"
#endif
}


/*
 * request downstream to present a client certificate during TLS handshake,
 * but does not validate it
 *
 * this is roughly equivalent to setting ssl_verify_client to optional_no_ca
 *
 * on success, NULL is returned, otherwise a static string indicating the
 * failure reason is returned
 */

const char *
ngx_http_lua_kong_ffi_request_client_certificate(ngx_http_request_t *r)
{
#if (NGX_SSL)
    return ngx_lua_kong_ssl_request_client_certificate(r->connection);
#else
    return "TLS support is not enabled in Nginx build"
#endif
}


int
ngx_http_lua_kong_ffi_get_full_client_certificate_chain(ngx_http_request_t *r,
    char *buf, size_t *buf_len)
{
#if (NGX_SSL)
    return ngx_lua_kong_ssl_get_full_client_certificate_chain(r->connection, buf, buf_len);
#else
    return NGX_ABORT;
#endif
}


#if (NGX_HTTP_SSL)

/*
 * called by ngx_http_upstream_ssl_init_connection right after
 * ngx_ssl_create_connection to override any parameters in the
 * ngx_ssl_conn_t before handshake occurs
 *
 * c->ssl is guaranteed to be present and valid
 */

void
ngx_http_lua_kong_set_upstream_ssl(ngx_http_request_t *r, ngx_connection_t *c)
{
    ngx_http_lua_kong_ctx_t     *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_lua_kong_module);

    if (ctx == NULL) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "skip overriding upstream SSL configuration, "
                       "module ctx not set");
        return;
    }

    return ngx_lua_kong_ssl_set_upstream_ssl(ctx->ssl_ctx, c);
}


int
ngx_http_lua_kong_ffi_set_upstream_client_cert_and_key(ngx_http_request_t *r,
    void *_chain, void *_key)
{
    ngx_http_lua_kong_ctx_t     *ctx;

    ctx = ngx_http_lua_kong_get_module_ctx(r);

    return ngx_lua_kong_ssl_set_upstream_client_cert_and_key(ctx->ssl_ctx, _chain, _key);
}


int
ngx_http_lua_kong_ffi_set_upstream_ssl_trusted_store(ngx_http_request_t *r,
    void *_store)
{
    X509_STORE                  *store = _store;
    ngx_http_lua_kong_ctx_t     *ctx;

    if (store == NULL) {
        return NGX_ERROR;
    }

    ctx = ngx_http_lua_kong_get_module_ctx(r);
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    return ngx_lua_kong_ssl_set_upstream_ssl_trusted_store(ctx->ssl_ctx,_store);
}


int
ngx_http_lua_kong_ffi_set_upstream_ssl_verify(ngx_http_request_t *r,
    int verify)
{
    ngx_http_lua_kong_ctx_t     *ctx;

    ctx = ngx_http_lua_kong_get_module_ctx(r);
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    return ngx_lua_kong_ssl_set_upstream_ssl_verify(ctx->ssl_ctx, verify);
}


int
ngx_http_lua_kong_ffi_set_upstream_ssl_verify_depth(ngx_http_request_t *r,
    int depth)
{
    ngx_http_lua_kong_ctx_t     *ctx;

    ctx = ngx_http_lua_kong_get_module_ctx(r);
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    return ngx_lua_kong_ssl_set_upstream_ssl_verify_depth(ctx->ssl_ctx, depth);
}


ngx_flag_t
ngx_http_lua_kong_get_upstream_ssl_verify(ngx_http_request_t *r,
    ngx_flag_t proxy_ssl_verify)
{
    ngx_http_lua_kong_ctx_t     *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_lua_kong_module);

    return ngx_lua_kong_ssl_get_upstream_ssl_verify(ctx->ssl_ctx, proxy_ssl_verify);
}


#endif


