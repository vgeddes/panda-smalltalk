"
Copyright (c) 2008 Vincent Geddes

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

OrderedCollection classMethod!
new
	^ self basicNew initialize: self defaultCapacity!

OrderedCollection classMethod!
new: requestedCapacity
	^ self basicNew initialize: requestedCapacity!

OrderedCollection classMethod!
minimumCapacity
	^ 8!

OrderedCollection classMethod!
defaultCapacity
	^ 8!

OrderedCollection method!
at: index put: anObject
	index > size
		ifTrue: [ self error: 'no object at this index' ].	
	^ array at: index put: anObject!

OrderedCollection method!
at: index
	index > size
		ifTrue: [ self error: 'no object at this index' ].
	^ array at: index!

OrderedCollection method!
add: anObject
	self ensureCapacity: size + 1.
	size := size + 1.
	array at: size put: anObject.
	^ anObject!

OrderedCollection method!
addAll: aCollection
	self ensureCapacity: size + aCollection size.
	aCollection
		do: [ :item |
			  array at: (size := size + 1) put: item ].
	
	^ aCollection!


OrderedCollection method!
removeAt: index
	| object |

	object := self at: index.
	self removeIndex: index.
	^ object!


OrderedCollection method!
removeIndex: index
	array replaceFrom: index
		  to: size - 1
		  with: array
	      startingAt: index + 1. 
	size := size - 1!

OrderedCollection method!
remove: anObject ifAbsent: aBlock
	| i |
	i := 0.
	[ (i := i + 1) <= size ]
		whileTrue:
			[anObject = (array at: i)
				ifTrue: [ self removeIndex: i. ^ anObject]].
				
	^ aBlock value!

OrderedCollection method!
removeFirst
	^ self removeAt: 0!

OrderedCollection method!
removeLast
	^ self removeAt: size!

OrderedCollection method!
do: aBlock
	| i |
	i := 0.
	[ (i := i + 1) <= size ]
		whileTrue: [ aBlock value: (array at: i) ]!

OrderedCollection method!
size
	^ size!

OrderedCollection method!
capacity
	^ array size!

OrderedCollection method!
ensureCapacity: requiredCapacity
	
	| newArray growSize |

	requiredCapacity <= array size
		ifTrue: [ ^ self ].

	"grow capacity by at least 100%" 
	growSize := (array size + array size) max: requiredCapacity.
	newArray := Array new: growSize.
	newArray replaceFrom: 1
			 to: size
			 with: array
	         startingAt: 1. 

	array := newArray!

OrderedCollection method!
initialize: capacity
	array := Array new: (capacity max: self class minimumCapacity).
	size := 0!
