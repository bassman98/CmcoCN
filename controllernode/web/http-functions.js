import { ok, badRequest } from 'wix-http-functions';

export function get_softwareversions(request) {
  return ok({
    headers: {
      "Content-Type": "application/json",
      "Cache-Control": "no-store"
    },
    body: {
      controller: {
        version: "18",
        firmwareUrl: "https://58e1d785-19fc-40e4-9f72-67035440c440.usrfiles.com/ugd/58e1d7_98cc7b67aade4c32ad12866f15c48927.txt"
      },
      node: {
        version: "18",
        firmwareUrl: "https://58e1d785-19fc-40e4-9f72-67035440c440.usrfiles.com/ugd/58e1d7_ede6bad800144f5ca835b12b73e58b26.txt"
      }
    }
  });
}