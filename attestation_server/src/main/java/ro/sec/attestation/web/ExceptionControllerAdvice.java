package ro.sec.attestation.web;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ExceptionHandler;

@ControllerAdvice
public class ExceptionControllerAdvice {

    @ExceptionHandler({Exception.class})
    public ResponseEntity<String> handleNoSuchElementException(Exception exception) {
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body("");
    }
}
