"
Copyright (c) 2007 Luca Bruno

This file is part of Smalltalk YX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
"

!Dictionary class methodsFor: 'instance creation'!

new: size
    ^self basicNew: ((size \\ 2 = 0
			 ifTrue: [ size + 1 ]
			 ifFalse: [ size ]) max: 15)
! !

!Dictionary methodsFor: 'accessing'!

removeKey: aKey ifAbsent: exceptionHandler
    | index |
    index := self indexOf: aKey.
    (self basicAt: index) isNil
	ifTrue: [ ^exceptionHandler value ].
    self basicAt: index put: nil.
    ^aKey
!

removeKey: aKey
    ^self removeKey: aKey ifAbsent: [ NotFound signal: 'Key not found' ]
!

remove: anAssociation ifAbsent: exceptionHandler
    ^self removeKey: anAssociation key ifAbsent: exceptionHandler
!

remove: anAssociation
    ^self removeKey: anAssociation key
!

indexOf: aKey
    | h2 pos size |
    size := self basicSize.
    pos := 2 * (aKey hash \\ ((size - 1) quo: 2)).
    h2 := pos quo: 4.

    (h2 between: 0 and: 1)
	ifTrue: [ h2 := 2 ].
    (h2 \\ 2) isZero
	ifFalse: [ h2 := h2 - 1 ].

    1 to: size do: [ :i |
	| key |
	key := self basicAt: pos + 1.
	key = aKey | key isNil
	    ifTrue: [ ^pos + 1 ].
	pos := pos + h2.
	pos >= (size - 1)
	    ifTrue: [ pos := pos - (size - 1) ] ].
    self error: 'Dictionary is full'
!

at: aKey
    | index |
    index := self indexOf: aKey.
    (self basicAt: index) isNil
	ifTrue: [ NotFound signal: 'Key not found' ].
    ^self basicAt: index + 1
!

at: aKey put: aValue
    | index |
    index := self indexOf: aKey.
    self
	basicAt: index put: aKey;
	basicAt: index + 1 put: aValue.
    ^aValue
! !

!Dictionary methodsFor: 'enumerating'!

keysAndValuesDo: aBlock
    1 to: self basicSize by: 2 do: [ :i |
	 | key |
	 (key := self basicAt: i)
	     ifNotNil: [ aBlock value: key value: (self basicAt: i + 1) ] ]
!

associationsDo: aBlock
    self keysAndValuesDo: [ :key :value |
	aBlock value: key -> value ]
!

keysDo: aBlock
    1 to: self basicSize by: 2 do: [ :i |
	| key |
	(key := self basicAt: i)
	    ifNotNil: [ aBlock value: key ] ]
!

do: aBlock
    1 to: self basicSize by: 2 do: [ :i |
	(self basicAt: i)
	    ifNotNil: [ aBlock value: (self basicAt: i + 1) ] ]
!

collect: transformerBlock
    "Evaluate the block for each element of the receiver.
     The results of these evaluations are collected into a new collection"
    | coll |
    coll := self copyEmpty.
    self keysAndValuesDo: [ :key :value |
	coll at: key put: (transformerBlock value: value) ].
    ^coll
!

reject: discriminatorBlock
    "Return a collection of which elements are the ones that cause discriminatorBlock to return false"
    | coll |
    coll := self copyEmpty.
    self keysAndValuesDo: [ :key :value |
	(discriminatorBlock value: value)
	    ifFalse: [ coll at: key put: value ] ].
    ^coll
!

select: discriminatorBlock
    "Return a collection of which elements are the ones that cause discriminatorBlock to return true"
    | coll |
    coll := self copyEmpty.
    self keysAndValuesDo: [ :key :value |
	(discriminatorBlock value: value)
	    ifFalse: [ coll at: key put: value ] ].
    ^coll
! !
