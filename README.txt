This is a template of the extension for your GiST iSAX implementation.

Extensions are discussed in general in  https://www.postgresql.org/docs/devel/static/extend-extensions.html
and the PGXS build infrastructure for extensions
https://www.postgresql.org/docs/devel/static/extend-pgxs.html
This extension is for an index, and this form of extension is discussed more in https://www.postgresql.org/docs/devel/static/xindex.html

For a GiST extension you need to implement each of the support functions listed in https://www.postgresql.org/docs/devel/static/xindex.html#XINDEX-GIST-SUPPORT-TABLE and described in more detail in https://www.postgresql.org/docs/devel/static/gist-extensibility.html

You also need to ensure that you cater for each of the comparison operations, which have associated strategies. See, e.g., https://www.postgresql.org/docs/devel/static/xindex.html#XINDEX-RTREE-STRAT-TABLE

Provided for you are skeletons that partially implement the required functions and operators 
to perform distance comparisons between real[] arrays. Also in  gist_isax--1.0.sql and 
gist_isax.c are the start of the GiST functions, taken from https://www.postgresql.org/docs/9.5/static/gist-extensibility.html
The fetch function is not included because it is assumed that your leaf entries will be lossy representations of the timeseries records.

The aim is to allow searches for time series which are similar to some reference. Note that PostgreSQL only supports the use of indexes for operators, not functions. So we can't optimise for a query like

SELECT *
FROM my_data
WHERE array_distance(signal, :reference) < 0.5;

Instead we can use an approach like pg_trm (https://www.postgresql.org/docs/devel/static/pgtrgm.html):

SET gist_isax.distance_threshold = 0.5;
SELECT *
FROM my_data
WHERE signal % reference;

Here the '%' operator selects only instances of signal that have a Euclidean distance to reference of no more than the specified limit (in this case, of 0.5).

The iSAX paper gives an equation for the lower-bound Euclidean distance between the PAA representation of a time series and the iSAX representation of another: MINDIST_PAA_iSAX. This is pivotal to the use of an iSAX index for the '%' operator. You should assume that all time series are of the same length, but you are invited to consider how to deal with other situations.

The index leaf-level entries should contain an iSAX representation of the time series data. The iSAX representation depends upon a given word length (w) and cardinality (alpha). An iSAX signal can be lossily compressed to a lower cardinality.
