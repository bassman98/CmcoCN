import { ok, badRequest } from 'wix-http-functions';

export function get_softwareversions(request) {
  return ok({
    headers: {
      "Content-Type": "application/json",
      "Cache-Control": "no-store"
    },
    body: {
      controller: {
        version: "14",
        firmwareUrl: "https://58e1d785-19fc-40e4-9f72-67035440c440.usrfiles.com/ugd/58e1d7_46f1f3723d7d4c1b8487e3480687f658.txt"
      },
      node: {
        version: "14",
        firmwareUrl: "https://58e1d785-19fc-40e4-9f72-67035440c440.usrfiles.com/ugd/58e1d7_831213f93c73411e8e674384aec2bac0.txt"
      }
    }
  });
}