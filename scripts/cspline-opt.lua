local ffi = require 'ffi'
local cgsl = require 'gsl'
local max, abs = math.max, math.abs

local T = cgsl.gsl_interp_cspline

local ms = gdt.read_csv [[thermal-sio2.csv]]

local function cspline_interp(ipoints)
    local N = #ipoints
    local w = matrix.new(N, 1, |i| ms:get(ipoints[i], "wl"))
    local n = matrix.new(N, 1, |i| ms:get(ipoints[i], "n"))
    local k = matrix.new(N, 1, |i| ms:get(ipoints[i], "k"))
    local interp_n = ffi.gc(cgsl.gsl_interp_alloc(T, N), cgsl.gsl_interp_free)
    local interp_k = ffi.gc(cgsl.gsl_interp_alloc(T, N), cgsl.gsl_interp_free)
    local accel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
    cgsl.gsl_interp_init(interp_n, w.data, n.data, N)
    cgsl.gsl_interp_init(interp_k, w.data, k.data, N)
    return function(wx)
        local ne = cgsl.gsl_interp_eval(interp_n, w.data, n.data, wx, accel)
        local ke = cgsl.gsl_interp_eval(interp_k, w.data, k.data, wx, accel)
        return ne, ke
    end
end

local function subsampling_eval(ipoints, tol)
    local interp = cspline_interp(ipoints)
    for i = 1, #ms do
        local wi = ms:get(i, "wl")
        local ni, ki = ms:get(i, "n"), ms:get(i, "k")
        local ne, ke = interp(wi)
        if abs(ne - ni) > tol or abs(ke - ki) > tol then
            print("found point", i, ne, ni, ke, ki)
            return i
        end
    end
    print("WORKS")
end

local function interp_delta_score(interp, ipoints, ia, ib)
    local del_n, del_k = 0, 0
    for i = ipoints[ia], ipoints[ib] do
        local ni, ki = ms:get(i, "n"), ms:get(i, "k")
        local ne, ke = interp(ms:get(i, "wl"))
        del_n = max(del_n, abs(ne - ni))
        del_k = max(del_k, abs(ke - ki))
    end
    return max(del_n, del_k)
end

local function find_delta_optimal(ipoints, i, ka, kb)
    local kbest = ka
    local del = 1000
    for k = ka + 1, kb - 1 do
        ipoints[i] = k
        local interp = cspline_interp(ipoints)
        local kdel = interp_delta_score(interp, ipoints, i-1, i+1)
        if kdel < del then
            kbest = k
            del = kdel
        end
    end
    return kbest, del
end

local function insert_opt_point(ipoints, i)
    local ka, kb = ipoints[i], ipoints[i+1]
    table.insert(ipoints, i+1, ka+1)
    local kbest, del = find_delta_optimal(ipoints, i+1, ka, kb)
    print("adding point", kbest, ms:get(kbest, "wl"))
    ipoints[i+1] = kbest
end

local function add_new_point(ipoints, i_nok)
    local wi = ms:get(i_nok, "wl")
    for i = 1, #ipoints - 1 do
        local wa = ms:get(ipoints[i], "wl")
        local wb = ms:get(ipoints[i+1], "wl")
        if wa < wi and wb > wi then
            insert_opt_point(ipoints, i)
            break
        end
    end
end

local ipoints = {1, #ms}
insert_opt_point(ipoints, 1)
print(">> INITIAL", ipoints)

while true do
    local i_nok = subsampling_eval(ipoints, 3e-4)
    if not i_nok then break end
    add_new_point(ipoints, i_nok)
end
print("ipoints", #ipoints, ipoints)
local msr = gdt.new(#ipoints, {"wl", "n", "k"})
for i = 1, #ipoints do
    for j = 1, 3 do
        msr:set(i, j, ms:get(ipoints[i], j))
    end
end
print(msr)
local interp = cspline_interp(ipoints)
ms:col_append("n_interp", function(r)
    local n, k = interp(r.wl)
    return n
end)
ms:col_append("k_interp", function(r)
    local n, k = interp(r.wl)
    return k
end)
ms:col_append("n_resid", |r| r.n_interp - r.n)
ms:col_append("k_resid", |r| r.k_interp - r.k)
gdt.plot(ms, "n_resid ~ wl")
gdt.plot(ms, "k_resid ~ wl")
gdt.hist(ms, "n_resid")
gdt.hist(ms, "k_resid")

_G["ms"] = ms
_G["msr"] = msr