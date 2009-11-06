#! /usrbin/python

import subprocess
import numpy
import cStringIO

def Load_weighted_cloud(file):
    # Load data and resize if necessary
    a = numpy.loadtxt(file).copy();
    (npoints, dim) = a.shape
    a.resize((npoints, 4))

    f = cStringIO.StringIO()
    numpy.savetxt(f, a);
    return f.getvalue()

pctvolume = subprocess.Popen("./pctoffset",
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE);

(result, error) = pctvolume.communicate(Load_weighted_cloud("test.cloud"));

f = open("test.p", "w");
f.write(result);
f.close()
