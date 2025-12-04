# Exercise 1
1536 Bits

# Exercise 2
```math
E(M) \cdot  E(R) = (M^{e}\mod N) \cdot (R^{e}\mod N)\mod N = M^{e}\cdot R^{e} \mod N = (M\cdot R)^{e} \mod N = E(M*R)
```
```math
D(C) \cdot D(G) 
= (C^d \bmod N) \cdot (G^d \bmod N) \bmod N 
= (C^d \cdot G^d) \bmod N 
= (C \cdot G)^d \bmod N 
= D(C \cdot G).
```

# Exercise 3
- 1 Exponentiation with e 
- 1 modular invers
- 2 modular multiplication

# Exercise 4
Since $` M^{ \varphi(N)} = 1; M^{d}\bmod N = M^{d '} \bmod N `$ therfore: 
$` D(M)=M^{d'}\bmod N`$ \
64-128 Bit \
Penalty:
- Exponentiation with d' might take slightly longer
- 1 modular invers
- 2 modular multiplication