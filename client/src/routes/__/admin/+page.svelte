<script lang="ts">
	import { logo } from "$lib/images/base64";
    import {PUBLIC_APP_VERSION} from '$env/static/public';
</script>

<div class="px-4 pb-4">
    <img class="w-90 mb-2" src={logo} alt="">
    <h3 class="bg-muted p-2 rounded-sm w-fit">Version {PUBLIC_APP_VERSION}, source: <a target="_blank" href="https://github.com/yurixahri/LiliumFS">here</a></h3>
</div>