sdup:
pop a
psh a
psh a
gb
sswp:
pop a
pop b
psh a
psh b
gb
scmp:
pop a
pop b
cmp a b
gb
sprn:
pop a
prn a
gb
sinc:
pop a
inc a
psh a
gb
sdec:
pop a
dec a
psh a
gb
sadd:
pop a
pop b
add a b
psh a
gb
ssub:
pop a
pop b
sub a b
psh a
gb
smul:
pop a
pop b
mul a b
psh a
gb
sdiv:
pop a
pop b
div a b
psh a
gb
smod:
pop a
pop b
mod a b
psh a
gb
sshr:
pop a
pop b
shr a b
psh a
gb
sshl:
pop a
pop b
shl a b
psh a
gb
shlt:
pop a
hlt a
gb
sand:
pop a
pop b
and a b
psh a
gb
sxor:
pop a
pop b
xor a b
psh a
gb
sor:
pop a
pop b
or a b
psh a
gb
snot:
pop a
not a
psh a
gb
srot:
pop a
pop b
pop c
psh a
psh b
psh c
gb
srm:
pop a
xor a, a
gb
snrm:
pop a
mv b a
snrm_l:
jb srm
dec b
cmp b 0
jg snrm_l
gb
