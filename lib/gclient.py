# ex:ts=4:sw=4:sts=4:et
# -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
"""
BitBake "Fetch" gclient implementation

gclient fetcher support the SRC_URI in the following format:

    SRC_URI = "gclient://some.host/path/;optionA=xxx,optionB=xy"

Supported SRC_UIR options are:

- branch
  The git branch of the root gclient project. The default is
  "master".

- tag
  Git tag for gclient to retrieve.

- protocol
  The method to use for accessing the gclient core project. The default
   is https.

- project-name
  Pass --name command line argument to gclient. Not default is empty.

- rev
  Specify git commit id for gclient. Default is empty.

- deps-file
  Set the --deps-file command line argument for gclient.

"""

# Copyright (C) 2015 Zoltan Kuscsik <zoltan.kuscsik@linaro.org>
#
# Based on git.py which is:
# Copyright (C) 2005 Richard Purdie
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import os
import bb
from   bb    import data
from   bb.fetch2 import FetchMethod
from   bb.fetch2 import runfetchcmd
import string

class Gclient(FetchMethod):
    """Class to fetch a module or modules using gclient"""

    def __fetch_url(self, ud):
        return "%s://%s%s" % (ud.proto, ud.host, ud.path)

    def supports(self, ud, d):
        """
        Check to see if a given url can be fetched with gclient.
        """
        return ud.type in ["gclient"]

    def urldata_init(self, ud, d):
        """
        Init gclient
        """
        ud.proto = ud.parm.get('protocol', 'https')

        ud.branch = ud.parm.get('branch', 'master')
        ud.tag = ud.parm.get('tag','')
        ud.rev = ud.parm.get('rev','')

        ud.deps_file = ud.parm.get('deps-file', '')
        ud.project_name = ud.parm.get('project-name', '')

        if( ( ('tag' in ud.parm) + ('branch' in ud.parm) + ('res' in ud.parm)) > 1):
            raise bb.fetch2.ParameterError("Conflicting revision setting SRC_URI. Specify only one of the options: 'branch', 'tag', 'res'.", ud.url)

        dldir = d.getVar("DL_DIR", True)

        gclient_src_name = 'gclient_%s%s' % (ud.host.replace(':', '.'), ud.path.replace('/', '.').replace('*', '.'))

        ud.syncdir = os.path.join(dldir, gclient_src_name)
        ud.localfile = ud.syncdir

        if ud.tag:
            ud.revision =  "refs/tags/%s" % ud.tag
        elif ud.rev:
            ud.revision = ud.rev
        else:
            ud.revision = "refs/heads/%s" % ud.branch

    def download(self, ud, d):
        """Fetch url"""
        print "wtf....."
        if not os.path.exists(ud.localpath):
            bb.utils.mkdirhier(ud.localpath)
        os.chdir(ud.localpath)
        gclient_url = self.__fetch_url(ud)


        # Add optional parameters
        extra_params=[]
        if ud.deps_file:
            extra_params.append("--deps-file=%s" % ud.deps_file)

        if ud.project_name:
            extra_params.append("--name=%s" % ud.project_name)

        extra_args = string.join(extra_params, sep = " ")

        runfetchcmd("gclient config %s %s " % (gclient_url, extra_args), d)

        # Gclient parameters used
        # -f                    - force sync
        # -nohooks              - skip hooks execution at this time
        # --with_branch_heads   - gclient can't find a commit ID if it is not in a branch
        #
        # Note: We should enable --no-history after some more testing
        print "Downloading"
        runfetchcmd("gclient sync --with_branch_heads -f  --nohooks --revision %s" % ud.revision, d)

    def unpack(self, ud, destdir, d):
        """ Unpack the src to destdir """
        print "wtf unpacking "
        destsuffix = "gclient/"
        destdir = ud.destdir = os.path.join(destdir, destsuffix)
        # Use rsync for copying. Creating a tar package here would be very
        # time consuming.
        runfetchcmd("rsync -av %s/ %s/" %(ud.syncdir, destdir), d)
        return True

    def clean(self, ud, d):
        """ Clean the gclient directory """
        bb.utils.remove(ud.localpath, True)

    def localpath(self, ud, dr):
        return ud.syncdir
    def need_update(self, ud, d):
        return True

    def supports_srcrev(self):
        return False
