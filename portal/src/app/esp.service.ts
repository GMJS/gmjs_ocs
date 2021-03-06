import { Injectable } from '@angular/core';
import { BehaviorSubject } from 'rxjs';
import { environment } from 'src/environments/environment';
import { MatSnackBar } from '@angular/material/snack-bar';
import { Mode } from './mode.enum';

export class Message {
  value: string;
  action: string;
}

export class File {
  name: string;
  size: number;
}

@Injectable({
  providedIn: 'root'
})
export class EspService {
  public unitName$ = new BehaviorSubject<string>("");
  public ssid$ = new BehaviorSubject<string>("");
  public pwd$ = new BehaviorSubject<string>("");
  public siteData$ = new BehaviorSubject<string>("");
  public files$ = new BehaviorSubject<Array<File>>([]);
  public logs$ = new BehaviorSubject<Array<string>>([]);
  public versionArduino$ = new BehaviorSubject<string>("");
  public versionEsp$ = new BehaviorSubject<string>("");
  public connected$ = new BehaviorSubject<boolean>(false);
  public verifyFileProgress$ = new BehaviorSubject<number>(undefined);
  public operationInProgress$ = new BehaviorSubject<boolean>(false);
  public progress$ = new BehaviorSubject<number>(0);
  public progressMessage$ = new BehaviorSubject<string>(undefined);
  public mode$ = new BehaviorSubject<Mode>(Mode.esp8266);

  private webSocket: WebSocket;
  private connectTimer: number;
  private pinger: number;

  constructor(private snackBar: MatSnackBar) {
    this.connect();
  }

  private connect(): void {
    let hostName = window.location.hostname;
    if (!environment.production) {
      hostName = localStorage.getItem("serverip");
    }
    if (hostName) {
      this.webSocket = new WebSocket("ws://" + hostName + ":81");
      this.webSocket.onopen = (event) => this.onOpen(event);
      this.webSocket.onclose = (event) => this.onClose(event);
      this.webSocket.onmessage = (message) => this.onMessage(message);
    }
  }

  private onOpen(event: Event): void {
    clearTimeout(this.connectTimer);
    this.pinger = setInterval(() => {
      if (!this.connected$.value) {
        this.connected$.next(true);
      }
      this.sendMessage({
        action: "ping",
        value: null
      });
    }, 500);
  }

  private onClose(event: Event): void {
    this.connected$.next(false);
    clearTimeout(this.connectTimer);
    clearInterval(this.pinger);
    this.connectTimer = setTimeout(() => {
      this.connect();
    }, 2000);
  }

  private onMessage(message: MessageEvent) {
    let data: Message = JSON.parse(message.data);
    switch (data.action) {
      case "mode": 
        this.mode$.next(Mode[data.value]);
        break;
      case "valueUnitName":
        this.unitName$.next(data.value);
        break;
      case "valueSsid":
        this.ssid$.next(data.value);
        break;
      case "valuePwd":
        this.pwd$.next(data.value);
        break;
      case "valueSiteData":
        this.siteData$.next(data.value);
        break;
      case "valueFiles":
        this.files$.next(JSON.parse(data.value));
        break;
      case "versionArduino":
        this.versionArduino$.next(data.value);
        break;
      case "versionEsp":
        this.versionEsp$.next(data.value);
        break;
      case "verifyBegin":
        this.verifyFileProgress$.next(0);
        break;
      case "verifyProgress":
        this.verifyFileProgress$.next(parseInt(data.value));
        break;
      case "verifyEnd":
        this.verifyFileProgress$.next(undefined);
        break;
      case "opInProgress":
        this.operationInProgress$.next(data.value == 'true');
        break;
      case "progress":
        this.progress$.next(parseInt(data.value));
        break;
      case "progressMessage":
        this.progressMessage$.next(data.value);
        break;
      case "success":
        this.snackBar.open(data.value, undefined, {
          duration: 2000
        });
        break;
      case "error":
        this.snackBar.open(data.value, undefined, {
          duration: 5000
        });
        break;
      case "log":
        this.logs$.next([data.value, ...this.logs$.value]);
        break;
    }
  }

  private sendMessage(message: Message) {
    this.webSocket.send(JSON.stringify(message));
  }

  public setUnitName(unitName: string): void {
    if (!unitName.trim().length) {
      return;
    }
    let data: Message = {
      action: "setUnitName",
      value: unitName.trim()
    }
    this.sendMessage(data);
  }

  public setSsid(ssid: string): void {
    this.sendMessage({
      action: "setSsid",
      value: ssid
    });
  }

  public setPwd(ssid: string): void {
    this.sendMessage({
      action: "setPwd",
      value: ssid
    });
  }

  public setSiteData(ssid: string): void {
    this.sendMessage({
      action: "setSiteData",
      value: ssid
    });
  }

  public deleteFile(fileName: string): void {
    this.sendMessage({
      action: "deleteFile",
      value: fileName
    });
  }

  public verifyFile(fileName: string): void {
    this.sendMessage({
      action: "verifyFile",
      value: fileName
    });
  }

  public verifyFlash(fileName: string): void {
    this.sendMessage({
      action: "verifyFlash",
      value: fileName
    });
  }

  public programFlash(fileName: string): void {
    this.sendMessage({
      action: "programFlash",
      value: fileName
    });
  }

  public restartArduino(): void {
    this.sendMessage({
      action: "restartArduino",
      value: null
    });
  }

  public restartEsp(): void {
    this.sendMessage({
      action: "restartEsp",
      value: null
    });
  }

  public applyWiFi(): void {
    this.sendMessage({
      action: "applyWiFi",
      value: null
    });
  }

  public resetEsp(): void {
    this.sendMessage({
      action: "resetEsp",
      value: null
    });
  }
}