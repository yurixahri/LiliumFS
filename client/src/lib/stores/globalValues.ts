import { PUBLIC_rootApi, PUBLIC_rootUrl } from '$env/static/public';

export const rootApi: string = PUBLIC_rootApi
export const rootUrl: string = PUBLIC_rootUrl
export const EVENTS = {
    SEND: 0,
    UPLOAD_START: 1,
    UPLOAD_COMPLETED: 2,
    UPLOAD_ERROR: 3
} as const;