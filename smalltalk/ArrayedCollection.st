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

!ArrayedCollection class methodsFor: 'instance creation'!

with: anObject
    "Create a collection with a single element"
    ^(self new: 1)
        at: 1 put: anObject;
        yourself
!

with: firstObject with: secondObject
    "Create a collection with the two given elements"
    ^(self new: 2)
        at: 1 put: firstObject;
        at: 2 put: secondObject;
        yourself
!

new: aSize withAll: aCollection
    "Answer an arrayed collection of the given size with the elements of aCollection"
    ^(self new: aSize)
	replaceFrom: 1 to: (aSize min: aCollection size) with: aCollection;
	yourself
!

withAll: aCollection
    "Answer an arrayed collection with the same elements of aCollection"
    ^self new: aCollection size withAll: aCollection
! !

!ArrayedCollection methodsFor: 'basic'!

add: anObject
    self shouldNotImplement
!

size
    <primitive: 'Object_size'>
	self primitiveFailed
! !

!ArrayedCollection methodsFor: 'copying'!

copyFrom: startIndex to: stopIndex
    | coll i |
    stopIndex < startIndex ifTrue: [ ^self copyEmpty: 0 ].
    coll := self copyEmpty: (stopIndex - startIndex + 1).
    i := 0.
    self from: startIndex to: stopIndex do: [ :ea |
	coll at: (i := i + 1) put: ea ].
    ^coll
!

copyWith: anObject
    "Copy the entire collection and add anObject at the end of it"
    | coll |
    coll := self class new: self size + 1 withAll: self.
    coll at: coll size put: anObject.
    ^coll
!

reverse
    "Answer a collection copying the receiver's elements in the opposite order"
    | coll i |
    coll := self copyEmpty.
    i := 0.
    self reverseDo: [ :ea |
	coll at: (i := i + 1) put: ea ].
    ^coll
! !


!ArrayedCollection methodsFor: 'replacing'!

replaceFrom: startIndex to: stopIndex with: aCollection
    <primitive: 'ArrayedCollection_replaceFromToWith'>
	^super replaceFrom: startIndex to: stopIndex with: aCollection
! !

!ArrayedCollection methodsFor: 'enumerating'!

collect: transformerBlock
    "Evaluate the block for each element of the receiver.
     The results of these evaluations are collected into a new collection"
    | coll |
    coll := self copyEmpty.
    1 to: self size do: [ :i |
	coll at: i put: (transformerBlock value: (self at: i)) ].
    ^coll
!

reject: discriminatorBlock
    "Return a collection of which elements are the ones that cause discriminatorBlock to return false"
    | stream |
    stream := WriteStream on: self copyEmpty.
    self do: [ :ea |
	(discriminatorBlock value: ea)
	    ifTrue: [ stream nextPut: ea ] ].
    ^stream contents
!

select: discriminatorBlock
    "Return a collection of which elements are the ones that cause discriminatorBlock to return true"
    | stream |
    stream := WriteStream on: self copyEmpty.
    self do: [ :ea |
	(discriminatorBlock value: ea)
	    ifTrue: [ stream nextPut: ea ] ].
    ^stream contents
! !
