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

"Basic"

Class named: 'Object'
	  superclass: 'nil'
	  instanceVariableNames: ''!

Class named: 'UndefinedObject'
	  superclass: 'Object'
	  instanceVariableNames: ''!

Class named: 'Behavior'
	  superclass: 'Object'
	  instanceVariableNames: 'format superclass instanceSize methodDictionary instanceVariableNames'!

Class named: 'Class'
	  superclass: 'Behavior'
	  instanceVariableNames: 'name'!

Class named: 'Metaclass'
	  superclass: 'Behavior'
	  instanceVariableNames: 'instanceClass'!


"Collections"

Class named: 'Collection'
	  superclass: 'Object'
	  instanceVariableNames: ''!

Class named: 'HashedCollection'
	  superclass: 'Collection'
	  instanceVariableNames: 'size deleted array'!

Class named: 'Set'
	  superclass: 'HashedCollection'
	  instanceVariableNames: ''!

Class named: 'Dictionary'
	  superclass: 'HashedCollection'
	  instanceVariableNames: ''!

Class named: 'IdentitySet'
	  superclass: 'Set'
	  instanceVariableNames: ''!

Class named: 'IdentityDictionary'
	  superclass: 'Dictionary'
	  instanceVariableNames: ''!

Class named: 'Bag'
	  superclass: 'Collection'
	  instanceVariableNames: 'contents'!

Class named: 'SequenceableCollection'
	  superclass: 'Collection'
	  instanceVariableNames: ''!

Class named: 'ArrayedCollection'
	  superclass: 'SequenceableCollection'
	  instanceVariableNames: ''!

Class named: 'Array'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''!

Class named: 'ByteArray'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''!

Class named: 'WordArray'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''!

Class named: 'OrderedCollection'
	  superclass: 'SequenceableCollection'
	  instanceVariableNames: 'array size'!

"TODO: if FloatArray not declared, then vm exits with an unhelpful assertion"
Class named: 'FloatArray'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''!

Class named: 'Interval'
	  superclass: 'SequenceableCollection'
	  instanceVariableNames: 'start stop step'!

Class named: 'Association'
	  superclass: 'Object'
	  instanceVariableNames: 'key value'!

Class named: 'List'
	  superclass: 'SequenceableCollection'
	  instanceVariableNames: 'first last size'!

Class named: 'Link'
	  superclass: 'Object'
	  instanceVariableNames: 'value next'!


"Streams"

Class named: 'Stream'
	  superclass: 'Object'
	  instanceVariableNames: ''!

Class named: 'PositionableStream'
	  superclass: 'Stream'
	  instanceVariableNames: 'collection position readLimit'!

Class named: 'WriteStream'
	  superclass: 'PositionableStream'
	  instanceVariableNames: 'writeLimit'!

Class named: 'FileStream'
	  superclass: 'WriteStream'
	  instanceVariableNames: 'fdesc mode'!


"Execution"

Class named: 'ContextPart'
	  superclass: 'Object'
	  instanceVariableNames: 'sender ip sp'!

Class named: 'MethodContext'
	  superclass: 'ContextPart'
	  instanceVariableNames: 'method receiver'!

Class named: 'BlockContext'
	  superclass: 'ContextPart'
	  instanceVariableNames: 'initialIP argcount caller home'!

Class named: 'CompiledMethod'
	  superclass: 'Object'
	  instanceVariableNames: 'header bytecode literals selector'!

Class named: 'Message'
	  superclass: 'Object'
	  instanceVariableNames: 'selector arguments'!


"Strings"

Class named: 'String'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''!

Class named: 'Symbol'
	  superclass: 'String'
	  instanceVariableNames: ''!

Class named: 'ByteString'
	  superclass: 'String'
	  instanceVariableNames: ''!

Class named: 'ByteSymbol'
	  superclass: 'Symbol'
	  instanceVariableNames: ''!

Class named: 'WideString'
	  superclass: 'String'
	  instanceVariableNames: ''!


"Numbers"

Class named: 'Magnitude'
	  superclass: 'Object'
	  instanceVariableNames: ''!

Class named: 'Number'
	  superclass: 'Magnitude'
	  instanceVariableNames: ''!

Class named: 'Integer'
	  superclass: 'Number'
	  instanceVariableNames: ''!

Class named: 'SmallInteger'
	  superclass: 'Integer'
	  instanceVariableNames: ''!

Class named: 'LargeInteger'
	  superclass: 'Integer'
	  instanceVariableNames: ''!

Class named: 'Fraction'
	  superclass: 'Number'
	  instanceVariableNames: 'numerator denominator'!

Class named: 'Float'
	  superclass: 'Number'
	  instanceVariableNames: ''!


"Basic data types"

Class named: 'Character'
	  superclass: 'Magnitude'
	  instanceVariableNames: ''!

Class named: 'Boolean'
	  superclass: 'Object'
	  instanceVariableNames: ''!

Class named: 'True'
	  superclass: 'Boolean'
	  instanceVariableNames: ''!

Class named: 'False'
	  superclass: 'Boolean'
	  instanceVariableNames: ''!


"System"

Class named: 'System'
	  superclass: 'Object'
	  instanceVariableNames: 'globals symbols'!