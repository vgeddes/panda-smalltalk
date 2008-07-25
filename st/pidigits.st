
"
* The Computer Language Shootout
    http://shootout.alioth.debian.org/
    contributed by Isaac Gouy
    modified by Eliot Miranda *

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of 'The Computer Language Benchmarks Game' nor the name of 'The Computer Language Shootout Benchmarks' nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

PiDigitSpigot method!
consume: aTransformation
   ^z * aTransformation!

PiDigitSpigot method!
digit
   ^(z extract: 3) floor!

PiDigitSpigot method!
isSafe: aDigit
   ^ aDigit = (z extract: 4) floor!

PiDigitSpigot method!
produce: anInteger
   inverse q: 10 r: -10 * anInteger s: 0 t: 1.
   ^inverse * z!

PiDigitSpigot method!
next
   | y |
   ^(self isSafe: (y := self digit))
      ifTrue: [z := self produce: y. self assert: [ y ~= 0 ]. y]
      ifFalse: [z := self consume: x next. y := self next. self assert: [ y ~= 0 ]. y]!

PiDigitSpigot method!
initialize
   z := Transformation unity.
   x := Transformation new.
   inverse := Transformation new.!

Number class method!
pidigitsTo: v width: width to: output
   | n i pidigits |
   n := v.
   i := 0.
   pidigits := PiDigitSpigot new.
   [n > 0] whileTrue:
      [n < width
         ifTrue:
            [n timesRepeat: [output nextPutAll: pidigits next printString].
             n to: width do: [:each | output nextPut: Character space].
             i := i + n]
         ifFalse:
            [width timesRepeat: [output nextPutAll: pidigits next printString].
             i := i + width].

      output nextPut: Character tab;
			 nextPut: $:;
			 nextPutAll: i printString;
             nextPut: Character cr.

      n := n - width]!

Transformation method!
* aTransformation
   ^ Transformation
      q: q * aTransformation q
      r: q * aTransformation r + (r * aTransformation t)
      s: s * aTransformation q + (t * aTransformation s)
      t: s * aTransformation r + (t * aTransformation t)!

Transformation method!
extract: anInteger
   ^(q * anInteger + r) // (s * anInteger + t)!

Transformation method!
next
   k := k + 1.
   q := k.
   r := 4 * k + 2.
   s := 0.
   t := 2 * k + 1.!

Transformation method!
q
   ^q!

Transformation method!
q: anInteger1 r: anInteger2 s: anInteger3 t: anInteger4
   q := anInteger1.
   r := anInteger2.
   s := anInteger3.
   t := anInteger4.
   k := 0.!

Transformation method!
r
   ^r!

Transformation method!
s
   ^s!

Transformation method!
t
   ^t!

Transformation method!
initialize
   q := 0.
   r := 0.
   s := 0.
   t := 0.
   k := 0.!

Transformation class method!
q: anInteger1 r: anInteger2 s: anInteger3 t: anInteger4
   ^(self basicNew) q: anInteger1 r: anInteger2 s: anInteger3 t: anInteger4!

Transformation class method!
unity
   ^self q: 1 r: 0 s: 0 t: 1!
