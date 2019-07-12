#!/bin/sh

# clean
./mach clobber

# init
./mach configure

TOP_OBJDIR=$(ls -td obj-* | head -1)
PYTHON_BIN="$TOP_OBJDIR/_virtualenvs/init/Scripts/python"  # windows
if [ ! -f "$PYTHON_BIN" ]; then
    PYTHON_BIN="$TOP_OBJDIR/_virtualenvs/init/bin/python"  # *nix
fi

# header files in objdir
$PYTHON_BIN ./init.py $TOP_OBJDIR
$PYTHON_BIN build/variables.py buildid_header > "$TOP_OBJDIR/buildid.h"
$PYTHON_BIN build/variables.py source_repo_header > "$TOP_OBJDIR/source-repo.h"

mkdir -p "$TOP_OBJDIR/config/external/ffi/.deps"  # bug

timeout --signal=9 30s ./mach build || echo  # bug

make -C "$TOP_OBJDIR" recurse_export

./mach build ./config/external/nspr/pr/
./mach build ./config/external/nspr/libc/
./mach build ./config/external/nspr/ds/
./mach build ./security/nss/lib/certdb/certdb_certdb/
./mach build ./security/nss/lib/certhigh/certhigh_certhi/
./mach build ./security/nss/lib/cryptohi/cryptohi_cryptohi/
./mach build ./security/nss/lib/base/base_nssb/
./mach build ./security/nss/lib/dev/dev_nssdev/
./mach build ./security/nss/lib/pki/pki_nsspki/
./mach build ./security/nss/lib/pk11wrap/pk11wrap_pk11wrap/
./mach build ./security/nss/lib/nss/nss_nss_static/
./mach build ./security/nss/lib/nss/nss_nss3_static/
./mach build ./db/sqlite3/src/
./mach build ./security/nss/lib/util/util_nssutil/
./mach build ./security/nss/lib/pkcs12/pkcs12_pkcs12/
./mach build ./security/nss/lib/pkcs7/pkcs7_pkcs7/
./mach build ./security/nss/lib/smime/smime_smime/
./mach build ./security/nss/lib/smime/smime_smime3_static/
./mach build ./security/nss/lib/ssl/ssl_ssl/
./mach build ./memory/build/
./mach build ./memory/mozalloc/
./mach build ./mozglue/misc/
./mach build ./ipc/mscom/mozglue/
./mach build ./mfbt || echo  # catch error
./mach build ./mozglue/build/
./mach build ./security/nss/

PLATFORM=$(echo $TOP_OBJDIR | sed 's/^obj-//')
DIST_DIR="dist/$PLATFORM/"
mkdir -p $DIST_DIR
cp $TOP_OBJDIR/dist/bin/nss3.dll $DIST_DIR
