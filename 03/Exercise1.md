# Exercise 1

### Partial Order

#### Reflexive
For any cache of type $ X $, a cache of type $ X $ trivially simulates itself: $ X \leq X $.

#### Transitive
If $ X \leq Y $ and $ Y \leq Z $, then $ X $ can be first modeled as $ Y $ and then as $ Z $. Therefore $ X \leq Z $.

#### Antisymmetric
Suppose $ X \leq Y $ and $ Y \leq X $. This implies that they have the same expressive capabilities, so they are the same type in modeling. Therefore $ X = Y $.

#### $ \mathrm{DMC} \leq \mathrm{FAC} $
Make an internal labeling of the FAC’s $ n $ lines to correspond to the $ n $ directly mapped slots.  
On a miss for block $ X $ whose DMC index would be $ i $, the FAC implementation places $ X $ into line $ i $.

#### $ \mathrm{DMC} \leq \mathrm{SAC} $
Consider a one-way SAC (1-way set associative). It behaves exactly like a DMC.

#### $ \mathrm{FAC} \leq \mathrm{SAC} $
Consider a SAC with one set and all lines in that set. It behaves exactly like a FAC.


### Hasse Diagram

SAC is the most general cache type.

SAC<br>
&nbsp;&nbsp;&nbsp;| <br> 
FAC<br>
&nbsp;&nbsp;&nbsp;| <br>
DMC<br>