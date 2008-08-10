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

ContextPart method!
sender
	^ sender!

ContextPart method!
ip
	^ ip!

ContextPart method!
sp
	^ sp!

ContextPart method!
printTraceBack
	| parent stream |
	
	stream := WriteStream on: (String new: 20).

	parent := self.
	[parent == nil]
		whileFalse:
			[parent printOn: stream.
			 (parent isMemberOf: BlockContext)
				ifTrue: [parent := parent caller]
				ifFalse: [parent := parent sender].
			  parent ifNotNil: [ stream nextPut: Character cr]].

	^ stream contents!


MethodContext method!
receiver
	^ receiver!

MethodContext method!
method
	^ method!


MethodContext method!
selector
	^ method selector!

MethodContext method!
printOn: aStream
	| class selector methodClass |
	
	class := self receiver class.
    methodClass := self method methodClass.
	selector := self selector.

	aStream nextPutAll: class name.
	(methodClass isNotNil and: [methodClass ~~ class])
		ifTrue: [aStream nextPutAll: '(', methodClass name, ')'].
	aStream nextPutAll: '>>'.
	aStream nextPutAll: selector!

BlockContext method!
argumentCount
	^ argcount!

BlockContext method!
caller
	^ caller!

BlockContext method!
home
	^ home!

BlockContext method!
method
	^ home method!

BlockContext method!
printOn: aStream
	aStream nextPutAll: home receiver class name.
	aStream nextPutAll: '>>'.
	aStream nextPutAll: home method selector.
	aStream nextPutAll: '[]'!
