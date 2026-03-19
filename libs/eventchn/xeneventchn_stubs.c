/*
 * Copyright (C) 2008-2014 Citrix Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <string.h>

#include <xenevtchn.h>

#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/threads.h>

static inline xenevtchn_handle *
xce_of_val (value v)
{
        return *(xenevtchn_handle **) Data_custom_val (v);
}

static void
stub_evtchn_finalize (value v)
{
        xenevtchn_close (xce_of_val (v));
}

static struct custom_operations xenevtchn_ops = {
        .identifier = "xenevtchn",
        .finalize = stub_evtchn_finalize,
        .compare = custom_compare_default,      /* Can't compare */
        .hash = custom_hash_default,    /* Can't hash */
        .serialize = custom_serialize_default,  /* Can't serialize */
        .deserialize = custom_deserialize_default,      /* Can't deserialize */
        .compare_ext = custom_compare_ext_default,      /* Can't compare */
};

CAMLprim value
stub_evtchn_init (value cloexec)
{
        CAMLparam1 (cloexec);
        CAMLlocal1 (result);
        xenevtchn_handle *xce;
        unsigned int flags = 0;

        if (!Bool_val (cloexec))
                flags |= XENEVTCHN_NO_CLOEXEC;

        result = caml_alloc_custom (&xenevtchn_ops, sizeof (xce), 0, 1);

        caml_release_runtime_system ();
        xce = xenevtchn_open (NULL, flags);
        caml_acquire_runtime_system ();

        if (xce == NULL)
          {
                  caml_failwith (strerror (errno));
          }

        *(xenevtchn_handle **) Data_custom_val (result) = xce;
        CAMLreturn (result);
}

CAMLprim value
stub_evtchn_fd (value xce_val)
{
        CAMLparam1 (xce_val);
        xenevtchn_handle *xce = xce_of_val (xce_val);
        int fd;

        /* Don't drop the GC lock.  This is a simple read out of memory */
        fd = xenevtchn_fd (xce);
        if (fd == -1)
          {
                  caml_failwith (strerror (errno));
          }

        CAMLreturn (Val_int (fd));
}

CAMLprim value
stub_evtchn_notify (value xce_val, value port_val)
{
        CAMLparam2 (xce_val, port_val);
        xenevtchn_handle *xce = xce_of_val (xce_val);
        int rc;
        int port = Int_val (port_val);

        caml_release_runtime_system ();
        rc = xenevtchn_notify (xce, port);
        caml_acquire_runtime_system ();

        if (rc == -1)
          {
                  caml_failwith (strerror (errno));
          }

        CAMLreturn (Val_unit);
}

CAMLprim value
stub_evtchn_bind_interdomain (value xce_val, value domid_val,
                              value remote_port_val)
{
        CAMLparam3 (xce_val, domid_val, remote_port_val);
        xenevtchn_handle *xce = xce_of_val (xce_val);
        xenevtchn_port_or_error_t rc;
        int domid = Int_val (domid_val);
        int remote_port = Int_val (remote_port_val);

        caml_release_runtime_system ();
        rc = xenevtchn_bind_interdomain (xce, domid, remote_port);
        caml_acquire_runtime_system ();

        if (rc == -1)
          {
                  caml_failwith (strerror (errno));
          }

        CAMLreturn (Val_int (rc));
}

CAMLprim value
stub_evtchn_alloc_unbound (value xce_val, value remote_domid_val)
{
        CAMLparam2 (xce_val, remote_domid_val);
        xenevtchn_handle *xce = xce_of_val (xce_val);
        xenevtchn_port_or_error_t rc;
        int remote_domid = Int_val (remote_domid_val);

        caml_release_runtime_system ();
        rc = xenevtchn_bind_unbound_port (xce, remote_domid);
        caml_acquire_runtime_system ();
        if (rc == -1)
          {
                  caml_failwith (strerror (errno));
          }

        CAMLreturn (Val_int (rc));

}

CAMLprim value
stub_evtchn_virq_dom_exc (value unit)
{
        CAMLparam1 (unit);
        CAMLreturn (Val_int (VIRQ_DOM_EXC));
}

CAMLprim value
stub_evtchn_bind_virq (value xce_val, value virq_val)
{
        CAMLparam2 (xce_val, virq_val);
        xenevtchn_port_or_error_t rc;
        xenevtchn_handle *xce = xce_of_val (xce_val);
        int virq = Int_val (virq_val);

        caml_release_runtime_system ();
        rc = xenevtchn_bind_virq (xce, virq);
        caml_acquire_runtime_system ();
        if (rc == -1)
          {
                  caml_failwith (strerror (errno));
          }

        CAMLreturn (Val_int (rc));
}

CAMLprim value
stub_evtchn_unbind (value xce_val, value port_val)
{
        CAMLparam2 (xce_val, port_val);
        xenevtchn_handle *xce = xce_of_val (xce_val);
        int port = Int_val (port_val);
        int rc;

        caml_release_runtime_system ();
        rc = xenevtchn_unbind (xce, port);
        caml_acquire_runtime_system ();
        if (rc == -1)
          {
                  caml_failwith (strerror (errno));
          }

        CAMLreturn (Val_unit);
}

CAMLprim value
stub_evtchn_pending (value xce_val)
{
        CAMLparam1 (xce_val);
        CAMLlocal1 (generation);
        xenevtchn_port_or_error_t port;
        xenevtchn_handle *xce = xce_of_val (xce_val);

        generation = caml_alloc_tuple (2);

        caml_release_runtime_system ();
        port = xenevtchn_pending (xce);
        caml_acquire_runtime_system ();
        if (port == -1)
          {
                  caml_failwith (strerror (errno));
          }

        Store_field (generation, 0, Val_int (0));
        Store_field (generation, 1, Val_int (port));

        CAMLreturn (generation);
}

CAMLprim value
stub_evtchn_unmask (value xce_val, value port_val)
{
        CAMLparam2 (xce_val, port_val);
        xenevtchn_handle *xce = xce_of_val (xce_val);
        int port = Int_val (port_val);
        int rc;

        caml_release_runtime_system ();
        rc = xenevtchn_unmask (xce, port);
        caml_acquire_runtime_system ();
        if (rc == -1)
          {
                  caml_failwith (strerror (errno));
          }

        CAMLreturn (Val_unit);
}
