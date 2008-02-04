"
Copyright (c) 2007 Vincent Geddes
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

"Basic meta"

nil subclass: #Object
    instanceVariableNames: ''
    classVariableNames: ''!

Object subclass: #UndefinedObject
       instanceVariableNames: ''
       classVariableNames: ''!

Object subclass: #Behavior
       instanceVariableNames: 'superclass instanceSize methodDictionary instanceVariables'
       classVariableNames: ''!

Behavior subclass: #Class
	 instanceVariableNames: 'name classPool'
         classVariableNames: ''!

Behavior subclass: #Metaclass
	 instanceVariableNames: 'instanceClass'
         classVariableNames: ''!


"Collections"

Object subclass: #Collection
       instanceVariableNames: ''
       classVariableNames: ''!

Collection subclass: #HashedCollection
           instanceVariableNames: 'tally array'
           classVariableNames: ''!

HashedCollection subclass: #Set
		 instanceVariableNames: ''
		 classVariableNames: ''!

HashedCollection subclass: #Dictionary
		 instanceVariableNames: ''
		 classVariableNames: ''!

Collection subclass: #SequenceableCollection
	   instanceVariableNames: ''
           classVariableNames: ''!

SequenceableCollection subclass: #Interval
	   instanceVariableNames: 'start stop step'
	   classVariableNames: ''!

SequenceableCollection subclass: #ArrayedCollection
		       instanceVariableNames: ''
                       classVariableNames: ''!

ArrayedCollection subclass: #Array
		  instanceVariableNames: ''
                  classVariableNames: ''!

ArrayedCollection subclass: #Tuple
		  instanceVariableNames: ''
                  classVariableNames: ''!

ArrayedCollection subclass: #ByteArray
		  instanceVariableNames: ''
                  classVariableNames: ''!

SequenceableCollection subclass: #OrderedCollection
		       instanceVariableNames: 'tally array'
                       classVariableNames: ''!

"Execution"

Object subclass: #ContextPart
       instanceVariableNames: 'parent method stack sp ip receiver temporaries returnContext'
       classVariableNames: ''!

ContextPart subclass: #MethodContext
	    instanceVariableNames: ''
            classVariableNames: ''!

MethodContext subclass: #BlockContext
	    instanceVariableNames: 'outerContext handledException handlerBlock ensureBlock'
            classVariableNames: ''!

Object subclass: #Process
       instanceVariableNames: 'context suspended returnedObject next scheduled'
       classVariableNames: ''!

Object subclass: #Semaphore
       instanceVariableNames: 'waitingProcesses signals'
       classVariableNames: ''!

Object subclass: #CompiledCode
       instanceVariableNames: 'header bytecode literals'
       classVariableNames: ''!

CompiledCode subclass: #CompiledMethod
	     instanceVariableNames: ''
             classVariableNames: ''!

CompiledCode subclass: #CompiledBlock
	     instanceVariableNames: 'method'
	     classVariableNames: ''!

Object subclass: #BlockClosure
       instanceVariableNames: 'block definedContext'
       classVariableNames: ''!


"Management"

Object subclass: #ProcessorScheduler
       instanceVariableNames: 'processByteslice firstProcess activeProcess'
       classVariableNames: ''!

Object subclass: #ObjectMemory
       instanceVariableNames: ''
       classVariableNames: ''!

"Streams"

Object subclass: #Stream
       instanceVariableNames: 'accessMode'
       classVariableNames: ''!

Stream subclass: #PositionableStream
       instanceVariableNames: 'collection curPos endPos'
       classVariableNames: ''!

PositionableStream subclass: #ReadStream
		   instanceVariableNames: ''
		   classVariableNames: ''!

PositionableStream subclass: #WriteStream
		   instanceVariableNames: ''
                   classVariableNames: ''!

WriteStream subclass: #ReadWriteStream
	    instanceVariableNames: ''
            classVariableNames: ''!

ReadWriteStream subclass: #ByteStream
		instanceVariableNames: ''
                classVariableNames: ''!

ByteStream subclass: #FileStream
	   instanceVariableNames: 'fd name filePtr peek'
           classVariableNames: ''!

FileStream subclass: #StdIOStream
	   instanceVariableNames: ''
	   classVariableNames: ''!

"Basic data types"

ByteArray subclass: #String
	  instanceVariableNames: ''
          classVariableNames: ''!

String subclass: #Symbol
       instanceVariableNames: ''
       classVariableNames: ''!

Object subclass: #Magnitude
       instanceVariableNames: ''
       classVariableNames: ''!

Magnitude subclass: #Character
	  instanceVariableNames: ''
          classVariableNames: 'WhiteSpaceTable XDigitsTable'!

Magnitude subclass: #Number
	  instanceVariableNames: ''
          classVariableNames: ''!

Magnitude subclass: #CPointer
	  instanceVariableNames: ''
          classVariableNames: ''!

Magnitude subclass: #DateTime
	  instanceVariableNames: 'offset second minute hour mday month year wday yday'
	  classVariableNames: ''!

Magnitude subclass: #Duration
	  instanceVariableNames: 'seconds'
	  classVariableNames: ''!

Number subclass: #Integer
       instanceVariableNames: ''
       classVariableNames: ''!

Integer subclass: #SmallInteger
	instanceVariableNames: ''
	classVariableNames: ''!

Integer subclass: #LargeInteger
	instanceVariableNames: ''
	classVariableNames: ''!

Number subclass: #Fraction
       instanceVariableNames: 'numerator denominator'
       classVariableNames: ''!

Number subclass: #Float
       instanceVariableNames: ''
       classVariableNames: ''!

Object subclass: #Boolean
       instanceVariableNames: ''
       classVariableNames: ''!

Boolean subclass: #True
	instanceVariableNames: ''
        classVariableNames: ''!

Boolean subclass: #False
	instanceVariableNames: ''
        classVariableNames: ''!

"Dictionaries"

Object subclass: #Association
       instanceVariableNames: 'key value'
       classVariableNames: ''!


Dictionary subclass: #IdentityDictionary
	   instanceVariableNames: ''
	   classVariableNames: ''!

IdentityDictionary subclass: #SystemDictionary
		   instanceVariableNames: 'systemInitializationRequests commandLineArguments startupProcess startupObject continueStartup'
		   classVariableNames: ''!

"Exceptions"

Object subclass: #Signal
       instanceVariableNames: 'handlerContext signalerContext messageText'
       classVariableNames: ''!

Signal subclass: #Exception
       instanceVariableNames: ''
       classVariableNames: ''!

Exception subclass: #Error
	  instanceVariableNames: ''
          classVariableNames: ''!

Error subclass: #NotImplemented
      instanceVariableNames: ''
      classVariableNames: ''!

Error subclass: #VMError
      instanceVariableNames: ''
      classVariableNames: ''!

Error subclass: #Halt
      instanceVariableNames: ''
      classVariableNames: ''!

Error subclass: #ZeroDivide
      instanceVariableNames: ''
      classVariableNames: ''!

VMError subclass: #PrimitiveFailed
	instanceVariableNames: ''
        classVariableNames: ''!

VMError subclass: #SystemSignal
	instanceVariableNames: ''
	classVariableNames: ''!

PrimitiveFailed subclass: #WrongArgumentCount
		instanceVariableNames: ''
		classVariableNames: ''!

Halt subclass: #MessageNotUnderstood
     instanceVariableNames: 'message receiver'
     classVariableNames: ''!

NotImplemented subclass: #NotYetImplemented
	       instanceVariableNames: ''
               classVariableNames: ''!

NotImplemented subclass: #ShouldNotImplement
	       instanceVariableNames: ''
               classVariableNames: ''!

ShouldNotImplement subclass: #SubclassResponsibility
		   instanceVariableNames: ''
                   classVariableNames: ''!

Error subclass: #InvalidValue
      instanceVariableNames: ''
      classVariableNames: ''!

InvalidValue subclass: #InvalidArgument
	     instanceVariableNames: ''
             classVariableNames: ''!

InvalidArgument subclass: #NotFound
		instanceVariableNames: ''
                classVariableNames: ''!

SystemSignal subclass: #UserInterrupt
	     instanceVariableNames: ''
	     classVariableNames: ''!

SystemSignal subclass: #TerminationSignal
	     instanceVariableNames: ''
	     classVariableNames: ''!

SystemSignal subclass: #AbnormalTermination
	     instanceVariableNames: ''
	     classVariableNames: ''!

SystemSignal subclass: #FloatingPointException
	     instanceVariableNames: ''
	     classVariableNames: ''!
