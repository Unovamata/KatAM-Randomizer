# find_symbols.py <katam decomp> <KatAM ROM> <symbols...>: the addresses of KatAM USA symbols (from the decompilation) in another
# version of KatAM, for "build.sh" to link the NiDL enemies against it; prints "name address" lines (see versions/*.txt).
# Functions are found by their code (BL targets and literal pool words masked); data through the literal pools of matched
# functions; what's left (library code, tables) by the shift of the nearest symbol found, checked against the code.
# Also prints what KatAMExpansion.cs patches (SpriteTables, SpawnTable, ObjectParameters2, HitFlashBranch, NameSite1/2, AnimCount);
import sys, subprocess, struct
D=sys.argv[1]
U=open(D+'/katam.gba','rb').read(); O=open(sys.argv[2],'rb').read()
syms={}; sizes={}; kinds={}
for l in subprocess.run(['arm-none-eabi-nm','-S',D+'/katam.elf'],capture_output=True,text=True).stdout.splitlines():
    p=l.split()
    if len(p)==4: syms[p[3]]=int(p[0],16); sizes[p[3]]=int(p[1],16); kinds[p[3]]=p[2]
    elif len(p)==3: syms.setdefault(p[2],int(p[0],16))
funcs=sorted((a&~1,n) for n,a in syms.items() if 0x08000000<=a<0x08400000 and n in sizes and sizes[n]>=16 and kinds.get(n,'?') in 'tT')
def mask(addr,size):
    b=bytearray(U[addr-0x08000000:addr-0x08000000+size]); m=bytearray(b'\xff'*size); lits=[]
    i=0
    while i+1<size:
        h=b[i]|b[i+1]<<8
        if (h>>11)==0x1E and i+3<size: m[i:i+4]=b'\0\0\0\0'; i+=4; continue
        if (h>>11)==0x09:  # ldr rX,[pc,#imm]
            lit=((addr+i+4)&~3)+(h&0xFF)*4-addr
            if 0<=lit<=size-4: m[lit:lit+4]=b'\0\0\0\0'; lits.append(lit)
        i+=2
    return bytes(b),bytes(m),lits
cache={}
def find(addr,size):
    if (addr,size) in cache: return cache[(addr,size)]
    b,m,lits=mask(addr,size)
    # anchor on the longest unmasked run;
    best=(0,0); s=None
    for i in range(size+1):
        if i<size and m[i]: s=i if s is None else s
        else:
            if s is not None and i-s>best[1]-best[0]: best=(s,i)
            s=None
    a0,a1=best
    if a1-a0<8: cache[(addr,size)]=None; return None
    hits=[]; st=0; pat=b[a0:a1]
    while True:
        k=O.find(pat,st)
        if k<0: break
        base=k-a0
        if base>=0 and all((O[base+j]==b[j]) or not m[j] for j in range(size)): hits.append(base)
        st=k+1
    r=(0x08000000+hits[0],lits) if len(hits)==1 else None
    cache[(addr,size)]=r; return r
want=[]
for arg in sys.argv[3:]:
    if arg.startswith('code:'):   # code:name=addr:size, a block of code matched like a function;
        n,v=arg[5:].split('='); a,sz=v.split(':'); syms[n]=int(a,16)|1; sizes[n]=int(sz,16); kinds[n]='t'
    elif '=' in arg: n,v=arg.split('='); syms[n]=int(v,16); kinds[n]='d'
    else: n=arg
    want.append(n)
out={}
for n in want:
    a=syms[n]
    if 0x08000000<=a<0x08400000 and n in sizes and kinds.get(n,'?') in 'tT':
        r=find(a&~1,sizes[n]); out[n]=(r[0]|(a&1)) if r else None
    else: out[n]=None
# data symbols: through literals of matched functions;
need=[n for n in want if out[n] is None]
if need:
    targets={syms[n]:n for n in need}
    votes={n:{} for n in need}
    for fa,fn in funcs:
        if not need: break
        sz=sizes[fn]
        b,m,lits=mask(fa,sz)
        hit=[l for l in lits if struct.unpack_from('<I',b,l)[0] in targets or (struct.unpack_from('<I',b,l)[0]&~1) in targets]
        if not hit: continue
        r=find(fa,sz)
        if not r: continue
        base=r[0]-0x08000000
        for l in hit:
            v=struct.unpack_from('<I',b,l)[0]; n=targets.get(v) or targets.get(v&~1)
            ov=struct.unpack_from('<I',O,base+l)[0]
            votes[n][ov]=votes[n].get(ov,0)+1
    for n in need:
        if votes[n]:
            v=max(votes[n],key=votes[n].get); out[n]=v
            if len(votes[n])>1: print('#',n,'votes',{hex(k):c for k,c in votes[n].items()},file=sys.stderr)
# the rest: the shift of the nearest symbol found (code and data move in blocks), checked against the code when it's code;
known=sorted((syms[n],out[n]-syms[n]) for n in want if out[n] is not None and 0x08000000<=syms[n]<0x0A000000)
for n in want:
    a=syms[n]
    if out[n] is not None or not (0x08000000<=a<0x0A000000) or not known: continue
    shift=min(known,key=lambda k:abs(k[0]-a))[1]
    if kinds.get(n,'?') in 'tT' and n in sizes:
        b,m,_=mask(a&~1,max(sizes[n],4)); base=(a&~1)+shift-0x08000000
        if not all(O[base+j]==b[j] or not m[j] for j in range(len(b))): continue
    out[n]=a+shift; print('#',n,'by shift',hex(shift),file=sys.stderr)
missing=[n for n in want if out[n] is None]
if missing: sys.exit('not found: ' + ' '.join(missing))
for n in want: print(n, hex(out[n]))

# What KatAMExpansion.cs patches (not in the runs this makes of itself for single symbols);
import os
if os.environ.get('FIND_SYMBOLS_CHILD'): sys.exit(0)
os.environ['FIND_SYMBOLS_CHILD'] = '1'
def u32(r, a): return struct.unpack_from('<I', r, a)[0]
def refs(r, value):
    found, k = [], -1
    while True:
        k = r.find(struct.pack('<I', value), k + 1)
        if k < 0: return found
        found.append(k)
# The sprite tables (gSpriteTables' ROM copy) follow gSpriteTables in crt0's literal pool;
k = O.find(struct.pack('<I', syms['gSpriteTables']), 0, 0x2000); sprites = u32(O, k + 4)
anims = u32(O, sprites - 0x08000000) - 0x08000000; count = 0
while 0x08000000 <= u32(O, anims + count * 4) < 0x0A000000: count += 1
print('SpriteTables', hex(sprites)); print('AnimCount', count)
for name in ('gSpawnFuncTable1', 'gUnk_08352AD0'):
    r = subprocess.run([sys.executable, sys.argv[0], D, sys.argv[2], name], capture_output=True, text=True).stdout.split()
    print({'gSpawnFuncTable1': 'SpawnTable', 'gUnk_08352AD0': 'ObjectParameters2'}[name], r[1])
flash = subprocess.run([sys.executable, sys.argv[0], D, sys.argv[2], 'code:HitFlash=0x080857A0:0x94'], capture_output=True, text=True).stdout.split()
print('HitFlashBranch', hex((int(flash[1], 16) & ~1) + 0x4E))
# The HP bar name copies (USA 0x08034ED2, 0x08035B34): the 1st and 4th literals pointing at the names table are theirs, 0x52 and
# 0x48 bytes after them, and their code must match the USA one's; 0 when they don't (no names patch: Japan's names are images
# of Japanese text, and they point elsewhere);
sites = []
uses = refs(O, out['gUnk_08D60EE4']) if 'gUnk_08D60EE4' in out else []
for n, (literal, back, length) in enumerate(((0, 0x52, 28), (3, 0x48, 30))):
    site = uses[literal] - back if len(uses) == 5 else -1
    usa = 0x34ED2 if n == 0 else 0x35B34
    b, m, _ = mask(0x08000000 + usa, length)
    ok = site > 0 and all(O[site + j] == b[j] or not m[j] for j in range(length))
    print('NameSite%d' % (n + 1), hex(0x08000000 + site) if ok else '0x0')
