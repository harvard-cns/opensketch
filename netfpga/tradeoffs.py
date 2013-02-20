import math


# number of bytes, no error guarantees, why 3 tables?
def flow_size_distribution(sum_freq):
    M = sum_freq * 2
    r = 3
    M -= M%(2**r)
    m = M/(2**r)
    num_tables = r
    counters_per_table = m
    num_counters = r * m
    counter_bits = 32
    num_bits = num_counters * counter_bits
    num_bytes = num_bits/8.0
    # sram, tcam, hash range
    return (num_bytes, r, [M])

    # their numbers
    # M = 2^20 = 2^4 64K counters = 1024K
    # (base 4) = 4^2 64K counters
    # is mapped to 3 arrays of 64K counters each (192K total)
    # number of flows rnages up to 563K
    # okay, so total number of counters is ~ 2 * num_flows
    # but how do you decide m?
    
    # an array of M = 2^r m counters
    # is mapped to r+1 arrays of m counters each
    
    # hash packet to M .., get which table
    # e.g., hash value is 510*m, table 2
    # which of 5 counters to update? 
    # 510%5 = counter 0

    # e.g., M = 2^10 * 5 counters
    # is mapped to 11 arrays of 5 counters each
    # 1024 thru 513 [.., .., .., .., ..]
    # 512 thru 257  [.., .., .., .., ..]
    # 256 thru 129  [.., .., .., .., ..]
    # 128 thru 65   [.., .., .., .., ..]
    # 64 thru 33    [.., .., .., .., ..]
    # 32 thru 17
    # 16 thru 9
    # 8, 7, 6, 5
    # 4, 3
    # 2
    # 1


# number of bytes for
def entropy(err, sum_freq, delta, show=True, k_base = 2):
    m = sum_freq
    
    k_num = 4.0 * 1.414 * math.log(m, k_base)
    k_den = (err/17.0) * (err/17.0) * math.log(math.log(m, k_base) * (17.0/err), k_base)
    k = k_num/k_den
    counters_per_table = 8.0 * k / (err/17.0)
    num_tables = math.ceil((math.log((17.0* m/err), math.e)))
    num_levels = math.ceil(math.log((1.0*m)/k, 2))
    num_copies = math.ceil(math.log(1.0/delta, math.e))
    num_counters = (counters_per_table * num_tables) * num_levels * num_copies
    counter_bits = math.ceil(math.log(m, 2))
    num_bits = num_counters * counter_bits
    num_bytes = num_bits/8.0
    if show:
        print "(%f counters_per_table * %f num_tables) * %f num_levels * %f num_copies * %f counter_bits / 8.0 is %f B" % (counters_per_table, num_tables, num_levels, num_copies, counter_bits, num_bytes)
        print " k is %f" % k 
    
    return (num_bytes, num_levels, [int(math.ceil(counters_per_table*(2**num_levels)))]*int(num_tables))

def f_a(M,E,k,q):
    return (1 - math.e**(- E * k / M))**(k * q)

def k_star(M, d, q):
    k_star_num = (M - 1) * math.log(2, math.e)
    k_star_den = q + q/2.0
    return k_star_num/k_star_den

def f_s_ub(M, E, k, q):
    return f_a(M-1, E + 0.5, k, q)

# number of TCAM entries for 
def bloom_filter(S, p, M=288, k=8, q=1):    
    d_max = S
    d_min = 1
    d_mid = (d_min + d_max)/2

    while  d_max > d_min:
        d_mid = (d_min + d_max)/2
        p_cand = math.ceil(S/d_mid) * f_s_ub(M, d_mid, k, q)
        
        #print "d_max %d, d_min %d, d_mid %d, p_cand %f v/s p %f" % (d_max, d_min, d_mid, p_cand, p)

        if p_cand < p:
            d_min = d_mid+1
        else:
            d_max = d_mid
        
    p_cand = (S/d_mid) * f_s_ub(M, d_mid, k, q)

    num_per_entry = d_mid
    if p_cand >= p:
        num_per_entry = d_mid - 1

        
    return (0, math.ceil(S/num_per_entry), [M]*int(k))

# number of bytes for, not counting reversible
def heavy_hitters(rel_err, delta=0.05, thresh=0.01, counter_bits=32):
    err = rel_err * thresh 
    counters_per_table = math.ceil(math.e/err)
    num_tables = int(math.ceil(math.log(1.0/delta)))
    num_bits = counters_per_table * num_tables * counter_bits
    num_bytes = num_bits/8.0
    return (num_bytes, 0, [counters_per_table]*num_tables)
    pass

# number of bytes for, not counting reversible, def. 1% heavy changers
# estimated_count is within actual_count(1 +- rel_err) with prob. >= 1 - delta
# false positives, estimated_count > thresh * SUM, actual_count < thresh * SUM
# false negatives, ..
def change_detection(rel_err, delta=0.05, thresh=0.01, counter_bits=32):
    err = rel_err * thresh 
    counters_per_table = math.ceil(8/err)
    num_tables = math.ceil(4 * math.log(1.0/delta))
    num_bits = counters_per_table * num_tables * counter_bits
    num_bytes = num_bits/8.0
    return (num_bytes, 0, [counters_per_table]*int(num_tables))
    pass

# m: num_bits, r: abs_thresh
def bitmap_err(m=149, r=33):
    r = 1.0 * r
    num = math.e**(r/m) - r/m - 1.0
    den = r**2/m
    err = math.sqrt(num/den)
    return err

def get_m_fillup(r):
    # start from 2 bits?
    found = False
    for m in range(2, 10*r):
        r = 1.0 * r
        check = 5 * math.sqrt(math.e**(r/m) - r/m - 1)
        if m > check:
            found = True
            break
    if found:
        return m
    else:
        print "m_fillup not found for r %f" % r
        print "searched from 2 %f through r * 10 %f " % (2, r*10)
        return m


# each copy has log(r) bits, say c copies, then c = 0.78/err**2
# relative error
def pcsa_err(m=149, r=33):
    r = 1.0 * r
    num_copies = math.floor(m/math.ceil(math.log(r,2)))
    err = 0.78 / math.sqrt(num_copies)
    return err

# number of bits
def pcsa(rel_err=0.145, r=33):
    num_copies = math.ceil(0.78**2/(rel_err**2))
    num_bits = num_copies * math.ceil(math.log(r,2))
    return (num_bits, math.ceil(math.log(r,2)), [math.ceil(math.log(r,2)*num_copies)])
    
# number of bits
def bitmap(rel_err=0.06, r=33):
    m_fillup = get_m_fillup(r)
    m_max = 10 * r
    found = False

    for m in range (m_fillup, m_max):
        if bitmap_err(m, r) < rel_err:
            found = True
            break

    if found:
        return (m, 0, [m])
    else:
        print "bitmap size not found for rel_err %f and r %f" % (rel_err, r)
        print "searched from m_fillup %f through m_max %f " % (m_fillup, m_max)
        return (m_fillup, 0, [m_fillup])

# number of bytes for, not counting reversible
# given b (how much error) and delta (with what confidence)
# r depends on k?
# should sample c/k for k-superspreaders, and r

def superspreaders1_err(k, b, delta, N, C_ss):
    # solve optim problem
    # get m, M_cm
    # also err_ss
    pass

def superspreaders0(k, b, delta, N, err_ss):
    pass

def superspreaders1(k, b, delta, N, err_bm, err_cm):
    # c, r = f(b, delta)
    # samp_ratio = c/k
    # return superspreaders2(err_bm, err_cm, N, samp_ratio, r)
    pass

# number of bytes for, not counting reversible
def superspreaders2(err_bm, err_cm, N, samp_ratio, r):
    # say we're using a bitmap
    num_bits, btcam_entries, bhashes = bitmap(err_bm, r)
    total_bytes, tcam_entries, hashes = heavy_hitters(rel_err=1, thresh=err_cm,counter_bits=num_bits)
    return (total_bytes, 0, bhashes+hashes)


def main():
    NUM = 500000
    F0 = 100000

    err = 0.1
    delta = 0.05

    print "FSD for %d distinct flows" % F0
    print flow_size_distribution(F0)

    print "Entropy for err %f, delta %f, and %f total flows" %(err, delta, NUM)
    print entropy(err, NUM, delta, show=False, k_base = 2)

    S = 200
    for delta in [0.05, 0.2, 0.5]:
        print "Bloom Filter for storing %d addresses, f.p.p. %f" % (S, delta)
        print bloom_filter(S, delta, M=288, k=8, q=1)
        pass


    print "PCSA err %f, (delta,  estimate's s.d. is err, assume Gaussian, so confidence >68p.c.) for up to %d distinct senders" %(err,  F0)
    print pcsa(err, F0)

    thresh = 0.1
    print "%d p.c. Heavy hitters for err %f, delta %f, and %f total flows" %(thresh * 100, err, delta, NUM)
    print heavy_hitters(err, delta, thresh, counter_bits=32)

    print "%d p.c. Change detection for err %f, delta %f, and %f total flows" %(thresh * 100, err, delta, NUM)    
    print change_detection(err, delta=0.05, thresh=0.01, counter_bits=32)

    k = 200
    err_bm = 0.2
    err_cm = 0.00001
    # corresponds to b=2, delta=0.2
    c = 44.83
    samp_ratio = c/k
    r = 33
    print "%d-Superspreaders for err_bm %f, err_cm %f, delta %f, and %f estimated distinct flows, sampling ratio %f, r %f" % (k, err_bm, err_cm, delta, F0, samp_ratio, r)
    print superspreaders2(err_bm, err_cm, F0, samp_ratio, r)
