import { ok, badRequest } from 'wix-http-functions';

export function get_softwareversions(request) {
  return ok({
    headers: {
      "Content-Type": "application/json",
      "Cache-Control": "no-store"
    },
    body: {
      controller: {
        version: "21",
        firmwareUrl: ""
      },
      node: {
        version: "21",
        firmwareUrl: ""
      }
    }
  });
}