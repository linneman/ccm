;; AT Parser Hooks invoked by Telephony Control Daemon implemented in Tinyscheme
;; Refer to README for more detailed information
;;
;; summer 2017, peiker/ol
;;
;;
;; usage illustration:
;;
;; execute in two separate terminal sessions the following commands:
;;
;; socat stdio pty,raw,echo=0,link=/tmp/modem_tcm
;; socat stdio pty,raw,echo=0,link=/tmp/host_tcm
;;
;; The command strings will be exchanged between them. Additional AT commands
;; defined here will be discarded.


;; load routing library from script directory
(load (string-append (get-script-dir) "/routes.scm"))


;; define device names of host (via USB CDC-TCM) and modem device (AT interface modem)
(define host-tcm-dev "/tmp/host_tcm")
(define modem-tcm-dev "/tmp/modem_tcm")

;; define service port of ALSA Usecase Manager
(define ucm-port 5044)


;; define intercept messages from host e.g. to change playback volume
;; send those messages to ALSAUCM
(define-routes host-request-routes
  (msg-begins-with "ATL" (write-channel ucm-ch (string-append "set-volume " args)) ""))


;; request from USB CDC-TCM host
;; are handled by host-request routes and otherwise forwarded to modem channel
(define (host-tcm-request-handler s)
  (write-channel modem-tcm-ch (route host-request-routes s)))


;; requests respectively events from modem are forwarded to USB CDC-TCM host
(define (modem-tcm-event-handler s)
  (write-channel host-tcm-ch s))


;; events from alsa usecase manager are forwarded to USB CDC-TCM host
(define (ucm-event-handler s)
  (write-channel host-tcm-ch s))


;; terminate tcm daemon application in a clean way to check for memory leackages
(define (terminate)
  (close-channel host-tcm-ch)
  (close-channel modem-tcm-ch)
  (close-channel ucm-ch)
  (quit))


(define host-tcm-ch (make-dev-channel host-tcm-dev host-tcm-request-handler))
(define modem-tcm-ch (make-dev-channel modem-tcm-dev modem-tcm-event-handler))
(define ucm-ch (make-client-sock-channel "127.0.0.1" ucm-port ucm-event-handler))


;; (is-channel-open host-tcm-ch)
;; (write-channel host-tcm-ch "hello")
