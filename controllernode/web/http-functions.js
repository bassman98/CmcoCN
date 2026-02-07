import { ok, badRequest } from 'wix-http-functions';

export function get_softwareversions(request) {
  return ok({
    headers: {
      "Content-Type": "application/json",
      "Cache-Control": "no-store"
    },
    body: {
      controller: {
        version: "17",
        firmwareUrl: "https://58e1d785-19fc-40e4-9f72-67035440c440.usrfiles.com/ugd/58e1d7_e8be784dbf9c4005b0ee71c1dec9e5d1.txt"
      },
      node: {
        version: "17",
        firmwareUrl: "https://58e1d785-19fc-40e4-9f72-67035440c440.usrfiles.com/ugd/58e1d7_42918554957140c383a8fb63b10632cf.txt"
      }
    }
  });
}