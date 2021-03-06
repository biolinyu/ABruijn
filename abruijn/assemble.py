#(c) 2016 by Authors
#This file is a part of ABruijn program.
#Released under the BSD license (see LICENSE file)

"""
Runs assemble binary
"""

import subprocess
import logging
import os

from abruijn.utils import which

ASSEMBLE_BIN = "abruijn-assemble"
logger = logging.getLogger()


class AssembleException(Exception):
    pass


def check_binaries():
    if not which(ASSEMBLE_BIN):
        raise AssembleException("Assemble binary was not found. "
                                "Did you run 'make'?")
    try:
        devnull = open(os.devnull, "w")
        subprocess.check_call([ASSEMBLE_BIN, "-h"], stderr=devnull)
    except subprocess.CalledProcessError as e:
        raise AssembleException("Some error inside native {0} module: {1}"
                                .format(ASSEMBLE_BIN, e))


def assemble(args, out_file, log_file):
    logger.info("Assembling reads")

    cmdline = [ASSEMBLE_BIN, "-k", str(args.kmer_size), "-l", log_file,
               "-t", str(args.threads), "-v", str(args.min_overlap)]
    if args.debug:
        cmdline.append("-d")
    if args.min_kmer_count is not None:
        cmdline.extend(["-m", str(args.min_kmer_count)])
    if args.max_kmer_count is not None:
        cmdline.extend(["-x", str(args.max_kmer_count)])
    cmdline.extend([args.reads, out_file, str(args.coverage)])

    try:
        subprocess.check_call(cmdline)
    except (subprocess.CalledProcessError, OSError) as e:
        raise AssembleException("Error in assemble binary: " + str(e))
