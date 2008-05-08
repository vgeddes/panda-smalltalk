;;
;; st-mode.el: A simple Smalltalk editing mode
;;
;; Copyright (C) 2008 Vincent Geddes <vincent.geddes@gmail.com>
;;
;; Permission is hereby granted, free of charge, to any person obtaining a copy
;; of this software and associated documentation files (the "Software"), to deal
;; in the Software without restriction, including without limitation the rights
;; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;; copies of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:
;;
;; The above copyright notice and this permission notice shall be included in
;; all copies or substantial portions of the Software.
;; 
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
;; THE SOFTWARE.

;;
;; This mode is intended for editing Smalltalk file-outs that use the chunk syntax. 
;;
;; To install st-mode.el:
;;
;; 1. Put st-mode.el somewhere in your load-path
;; 2. Add the following line to your .emacs file:
;;    (add-to-list 'auto-mode-alist '("\\.st\\'" . st-mode))
;;    (autoload 'st-mode "st-mode" nil t)
;;

(require 'font-lock)

(defvar st-mode-hook nil)

(defvar st-mode-map
  (let ((st-mode-map (make-keymap)))
    (define-key st-mode-map "\C-j" 'newline-and-indent)
    st-mode-map)
  "Keymap for Smalltalk major mode")

;;;    ; "pseudo" keywords
;;;    '("\\(false\\|nil\\|s\\(?:elf\\|uper\\)\\|t\\(?:hisContext\\|rue\\)\\)" . font-lock-keyword-face)
;;;    ; character constants
;;;    '("\\(\$[]a-zA-Z0-9!@#$%^&*()\-_=+{};:,./<>?\\|]\\)" . font-lock-constant-face)
;;;    ; symbol constants
;;;    '("\#[a-zA-Z][a-zA-Z0-9]*" . font-lock-constant-face)
;;;    ; number constants
;;;    '("\\([0-9][0-9]r\\)?[0-9]\\(\.[0-9]\\(e-?[0-9]\\)?\\)?" . font-lock-constant-face))

(defvar st-font-lock-keywords
   `((,(rx symbol-start
	(or "false" "true" "nil" "self" "super" "thisContext")
	symbol-end)
      . font-lock-keyword-face)
     (,(rx (regexp "\\([0-9][0-9]r\\)?[0-9]\\(\.[0-9]\\(e-?[0-9]\\)?\\)?"))
      . font-lock-constant-face)
     (,(rx (regexp "\#[a-zA-Z][a-zA-Z0-9]*"))
      . font-lock-constant-face)
     (,(rx (regexp "\\(\$[]a-zA-Z0-9!@#$%^&*()\-_=+{};:,./<>?\\|]\\)"))
      . font-lock-constant-face))
     "Highlighting expressions for Smalltalk mode")


(defvar st-mode-syntax-table
  (let ((st-mode-syntax-table (make-syntax-table)))
    (modify-syntax-entry ?' "|" st-mode-syntax-table)
    (modify-syntax-entry ?\" "!" st-mode-syntax-table)
    st-mode-syntax-table)
  "Syntax table for Smalltalk mode") 

(defvar st-indent-level 4
  "Default indent level for indenting")

(defun st-indent-line ()
  "Indent current line as Smalltalk code"
  ; Allows unrestricted indenting of current line unless
  ; line happens to be the first in the buffer or a chunk.
  (interactive)
  (beginning-of-line)
  (if (bobp)
      (indent-line-to 0)
    (let ((can-indent t))
      (save-excursion
	(skip-chars-backward "\r\n\t ")
	(if (char-equal (preceding-char) ?!)
	    (setq can-indent nil)))
      (if can-indent
	  (indent-line-to (+ (current-indentation) st-indent-level))))))
  
(defun st-mode ()
  "Major mode for editing ANSI Smalltalk Interchange files"
  (interactive)
  (kill-all-local-variables)
  (set-syntax-table st-mode-syntax-table)
  (use-local-map st-mode-map)
  (set (make-local-variable 'font-lock-defaults) '(st-font-lock-keywords))
  (set (make-local-variable 'indent-line-function) 'st-indent-line)
  (set (make-local-variable 'comment-start) "\"")
  (set (make-local-variable 'comment-end) "\"")
  (set (make-local-variable 'comment-padding) nil)
  (set (make-local-variable 'comment-multi-line) t)
  (set (make-local-variable 'tab-width) st-indent-level)
  (setq major-mode 'st-mode)
  (setq mode-name "Smalltalk")
  (run-hooks 'st-mode-hook))

(provide 'st-mode)
