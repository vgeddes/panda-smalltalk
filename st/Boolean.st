"
Boolean.st

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


Boolean classMethod!
new
	self error: 'Boolean can only have two instances, true and false'!
Annotation key: 'category' value: 'logical operations'!


"logical operations"

Boolean method!
& aBoolean
	self subclassResponsibility!
Annotation key: 'category' value: 'logical operations'!

Boolean method!
| aBoolean
	self subclassResponsibility!
Annotation key: 'category' value: 'logical operations'!

Boolean method!
not
	self subclassResponsibility!
Annotation key: 'category' value: 'logical operations'!

Boolean method!
xor: aBoolean
	^ aBoolean ~= self!
Annotation key: 'category' value: 'logical operations'!


"controlling"

Boolean method!
ifTrue: trueBlock ifFalse: falseBlock
	self subclassResponsibility!
Annotation key: 'category' value: 'controlling'!

Boolean method!
ifFalse: falseBlock ifTrue: trueBlock
	self subclassResponsibility!
Annotation key: 'category' value: 'controlling'!

Boolean method!
ifFalse: falseBlock
	self subclassResponsibility!
Annotation key: 'category' value: 'controlling'!

Boolean method!
ifTrue: trueBlock
	self subclassResponsibility!
Annotation key: 'category' value: 'controlling'!


"testing"

Boolean method!
isBoolean
	^ true!
Annotation key: 'category' value: 'testing'!
 